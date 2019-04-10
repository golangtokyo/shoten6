= Goにおけるデータベース実践入門

== はじめに

freee株式会社でバックエンドエンジニアをしている@budougumi0617@<fn>{bd617_twitter}です。

//footnote[bd617_twitter][@<href>{https://twitter.com/budougumi0617}]

=== 本章の目的・ゴール
本章では、GoからRDBMS@<fn>{bd617_rdbms}（関係データベース管理システム、 @<tti>{Relational Database Management System}）を操作する実装やテストの作成方法を紹介します。なお、簡略化のため、本章では以降RDBMSのことを単にデータベースと記載します。


2019年になり、Goの日本語書籍は多く発売されています。また、インターネット上の日本語記事も多くなりました。しかし、データベースを使った実装、テストの方法の基本的な部分をまとめた入門用の文章はほとんどないかと思います。そこで本章では今までGoのコードを使ってデータベースを操作する実装・テストを行なったことがない方向けに、次の情報をまとめました@<fn>{bd_versions_note}。

 * Docker@<fn>{bd_docker}を利用したデータベースの準備方法
 * @<code>{github.com/rubenv/sql-migrate}@<fn>{bd_sql-migrate}を使ったデータベースのマイグレーション方法
 * リポジトリパターンを使ったデータベースアクセスと業務ロジックを疎にする設計方針
 * @<code>{github.com/DATA-DOG/go-sqlmock}@<fn>{bd_sql-mock}を使った単体テストの書き方
 * @<code>{database/sql}パッケージを使ったデータベース操作の入門
 * MySQLサーバを利用した結合テストの書き方
 * CircleCI@<fn>{bd_circleci}上でMySQLサーバを利用した自動テストを行なう設定例


なお、今回はデータベースとして一般的なMySQL@<fn>{bd617_mysql}を使います。
本章で利用する各種技術のバージョンは@<table>{bd_versions}の通りです。

//table[bd_versions][各種バージョン]{
対象技術	バージョン
----------------------------
OS	MacOS Mojave(10.14.3)
HomeBrew@<fn>{bd_brew}	2.1.0
Go	1.12.2 darwin/amd64
Docker for Mac	18.09.2 darwin/amd64
MySQL	5.7
CircleCI configuration	2.1
github.com/rubenv/sql-migrate	v1.0.0
github.com/go-sql-driver/mysql	v1.4.1
github.com/DATA-DOG/go-sqlmock	v1.3.3
//}

また、Goは環境変数として@<code>{GO111MODULE=on}を設定して、Moudleモード@<fn>{bd_modules}を有効にしています。
なお、以降のコマンドライン操作はすべてMacOS上でのみ動作確認をしています。WindowsやLinux環境をご利用の方は適宜コマンド操作を置き換えてください。

本文中で紹介するコードはGitHub上に公開されています。

 * @<href>{https://github.com/budougumi0617/go-sql-sample}

//footnote[bd617_rdbms][@<href>{https://en.wikipedia.org/wiki/Relational_database_management_system}]
//footnote[bd_versions_note][各技術・用語については以降の節で実際に利用する時に説明します。]
//footnote[bd_docker][@<href>{https://www.docker.com/}]
//footnote[bd_sql-migrate][@<href>{https://github.com/rubenv/sql-migrate}]
//footnote[bd_sql-mock][@<href>{https://github.com/DATA-DOG/go-sqlmock}]
//footnote[bd_circleci][@<href>{https://circleci.com/}]
//footnote[bd617_mysql][@<href>{https://www.mysql.com/}]
//footnote[bd_brew][@<href>{https://brew.sh}]
//footnote[bd_modules][@<href>{https://github.com/golang/go/wiki/Modules}]

== 事前準備
実際にGoのコードを書く前に事前準備をしましょう。まず、プロジェクトで利用するディレクトリを作ります。

//cmd{
# budougumi0617の部分は各自のGitHubアカウント名に置き換えてください。
$ mkdir -p $GOPATH/src/github.com/budougumi0617/go-sql-sample
//}

本章の以降の操作は、特に明記がない限りここで作成した@<code>{go-sql-sample}ディレクトリ内で行われます@<fn>{bd_gopath}。

//footnote[bd_gopath][Moduleモードを利用している場合は@<code>{$GOPATH}以下にプロジェクトを作成する必要はありませんが、@<code>{$GOPATH}以下の作成が禁止されたわけではありません。]

次に、環境変数として@<list>{bd_env}の環境変数を定義しておきます。

//list[bd_env][環境変数の設定]{
export GO111MODULE=on
export SHOTEN6_MYSQL_USER=root
export SHOTEN6_MYSQL_PORT=3306
//}

もし既にMySQLをローカルで起動していて、次節の方法でMySQLコンテナを利用する場合は、既存のMySQLとポートが競合してしまうので、@<code>{SHOTEN6_MYSQL_PORT}の値を別の値に変更してください（例：@<code>{43306}など）。

なお、必須作業ではありませんが、私は@<code>{go-sql-sample}ディレクトリに@<code>{.envrc}として@<list>{bd_env}の環境変数を保存しました。ファイルに保存した状態で@<code>{github.com/direnv/direnv}というツールを使うと、そのディレクトリでコマンドを実行するときだけ自動で環境変数が定義されるので便利です。次のコマンドラインの実行例は@<code>{direnv}を利用している状態で各ディレクトリで環境変数を確認した実行結果です。

//cmd{
$ cd ~
$ echo $MYSQL_PORT

$ cd $GOPATH/src/github.com/budougumi0617/go-sql-sample
direnv: loading .envrc
direnv: export +GO111MODULE +SHOTEN6_MYSQL_PORT +SHOTEN6_MYSQL_USER
$ echo $MYSQL_PORT
3306
//}

//footnote[bd_direnv][@<href>{https://github.com/direnv/direnv}]

ローカルにMySQLのクライアントツールがインストールされていない場合は、@<code>{mysql}コマンドもインストールしておきましょう。

//cmd{
$ brew install mysql
//}

また、MacでDockerを使う場合はDocker for Macが便利です。インストールページ@<fn>{bd_docker_mac}の手順に則ってDockerを利用できるようにしておきましょう。

//footnote[bd_docker_mac][@<href>{https://hub.docker.com/editions/community/docker-ce-desktop-mac}]

==={bd_hd_docker} Dockerを利用したデータベース環境の準備方法
環境変数の設定準備が完了したら、Goから接続するデータベースとしてMySQLサーバをローカルで起動しておきます。MySQLはSQL@<fn>{bd_sql}を扱うオープンソースで公開されているデータベースのひとつです。
ここでは、Dockerを使ってコンテナとしてMySQLサーバを起動します。Dockerコンテナとしてデータベースを起動することで、利用が終了したあと簡単に廃棄することができます。既にMySQLのデータベース環境をローカルに構築済みの方はその環境を利用してもかまいません。

//footnote[bd_sql][@<href>{https://ja.wikipedia.org/wiki/SQL}]

Dockerを利用してMySQLコンテナを起動するには次のコマンドを実行するだけで十分です。ローカルにDockerイメージがない場合は自動的にイメージのダウンロードが行われます。
コマンドオプションはデーモンとして起動、空パスワードの許可、環境変数@<code>{SHOTEN6_MYSQL_PORT}の値にポートフォワーディングするように設定し、
@<code>{mysql_tmp}という名前で起動させるという意味です。

//cmd{
$ docker run --rm -d -e MYSQL_ALLOW_EMPTY_PASSWORD=yes \
    -p $(SHOTEN6_MYSQL_PORT):3306 --name mysql_tmp mysql:5.7
//}

@<code>{docker run}コマンドでMySQLのコンテナを起動したあとは、@<code>{mysql}コマンドを使って動作確認をしておきましょう。
なお、コンテナ起動直後の数秒間はMySQLの初期化が行われているので、少し時間を置いてから次の@<code>{mysql}コマンドを実行してください。
デフォルトで作成されている管理用の@<code>{database}の情報が取得できたら接続成功です。

//cmd{
$ mysql -h 127.0.0.1 --port ${SHOTEN6_MYSQL_PORT} \
    -u${SHOTEN6_MYSQL_USER} -e "show databases;"
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| sys                |
+--------------------+
//}

なお、利用後は以下のコマンドでDockerコンテナを停止させることができます。

//cmd{
$ docker stop mysql_tmp
//}

== github.com/rubenv/sql-migrateを使ったデータベースのマイグレーション方法
ローカルにMySQLサーバを準備することができました。次はMySQLサーバにGoから操作するデータを保存するためのデータベースとテーブルを作成します。
一番簡単な方法は、@<code>{mysql}コマンドで直接SQLを実行することでしょうが、それではローカル環境以外で同様のデータベースを再現することができません。また、サービス開発をするならば新しいテーブルを追加したり、カラムを追加することもあるでしょう。そのため、ここではマイグレーションツールを使ってデータベースの状態を管理していきます。

今回はGo製のマイグレーションツールである@<code>{github.com/rubenv/sql-migrate}を使ってマイグレーションを管理します。利用したことがない場合は次の@<code>{go get }コマンドでローカルにインストールすることが出来ます。

//cmd{
$ go get -u github.com/rubenv/sql-migrate/sql-migrate
$ sql-migrate -h
Usage: sql-migrate [--version] [--help] <command> [<args>]

Available commands are:
    down      Undo a database migration
    new       Create a new migration
    redo      Reapply the last migration
    skip      Sets the database level to the most recent version available, \
        without running the migrations
    status    Show migration status
    up        Migrates the database to the most recent version available
//}

@<code>{sql-migrate}コマンドは予めMySQLサーバ上に@<code>{database}の定義が必要になるため、@<code>{CREATE DATABASE}を実行するSQLファイルだけ作成しておきます。次のコマンドで @<code>{db}ディレクトリと、あとでマイグレーション情報を保存するための@<code>{db/migrations}ディレクトリを作成し、今回利用する@<code>{sql_sample}データベースを作成するための@<code>{db/database.sql}を作成します。

//cmd{
$ mkdir -p db/migrations
$ cat << EOF > db/database.sql
CREATE DATABASE IF NOT EXISTS sql_sample;
EOF
//}

ここでSQL文を@<code>{db/database.sql}とファイルにしておくことで別環境での再現性を確保しておきます。@<code>{db/database.sql}を使ってデータベースを作成しましょう。

//cmd{
$ mysql -h 127.0.0.1 --port ${SHOTEN6_MYSQL_PORT} \
    -u${SHOTEN6_MYSQL_USER} < db/database.sql
//}

もう一度@<code>{mysql}コマンドで@<code>{SHOW DATABASES}を実行すれば、@<code>{sql_sample}データベースが作成されているのがわかります。

//cmd{
$ mysql -h 127.0.0.1 --port ${SHOTEN6_MYSQL_PORT} \
    -u${SHOTEN6_MYSQL_USER} -e "show databases;"
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| sql_sample         |
| sys                |
+--------------------+
//}

データベースを用意できたら、@<code>{sql-migrate}コマンド用のデータベースへの接続設定ファイル（@<code>{dbconfig.yml}）を作成します。
@<list>{bd_dbconfig}が設定ファイルの中身です@<fn>{dbconfig}。

//list[bd_dbconfig][./dbconfig.yml]{
development:
    dialect: mysql
    dir: db/migrations
    # 紙面の都合上改行をいれています。
    # 正しくは"...@tcp(127.0.0.1:${SHOTEN6_MYSQL_PORT})/sql_sample?..."とつながります。
    datasource: ${SHOTEN6_MYSQL_USER}:@tcp(127.0.0.1:${SHOTEN6_MYSQL_PORT})\
    /sql_sample?charset=utf8mb4&collation=utf8mb4_general_ci&parseTime=true

# Dummy setting
production:
    dialect: postgres
    dir: migrations/postgres
    datasource: dbname=myapp sslmode=disable
    table: migrations
//}

//footnote[dbconfig][@<href>{https://github.com/budougumi0617/go-sql-sample/blob/master/dbconfig.yml}]

@<code>{dbconfig.yml}を作成したら、一度@<code>{sql-migrate status}コマンドを実行してみましょう。何もマイグレーションファイルを作っていないので、特に情報は出力されませんが、次のような表示が出ればデータベースとの接続ができています。

//cmd{
$ sql-migrate status
+-----------+---------+
| MIGRATION | APPLIED |
+-----------+---------+
+-----------+---------+
//}

接続確認が済んだら、マイグレーション用のファイルを作成しましょう。@<code>{sql-migrate new}コマンドを実行すると、マイグレーション用のSQLファイルが生成されます。

//cmd{
$ sql-migrate new create_user
Created migration db/migrations/20190409140628-create_user.sql
//}

今回は@<list>{bd_entity}のような@<code>{User}情報をデータベースに保存することにします。

//list[bd_entity][Userの定義]{
type User struct {
  ID        int64     `db:"id"`
  Name      string    `db:"name"`
  Email     string    `db:"email"`
  CreatedAt time.Time `db:"created_at"`
  UpdatedAt time.Time `db:"updated_at"`
//}

@<list>{bd_entity}に対応する@<code>{user}情報を管理するテーブルを作成するマイグレーション定義が@<list>{bd_create_user}です。

//list[bd_create_user][./db/migrations/20190409140628-create_user.sql]{
-- Sample migration

-- +migrate Up
CREATE TABLE `user` (
`id` bigint unsigned PRIMARY KEY AUTO_INCREMENT,
`name` varchar(255) NOT NULL COMMENT 'user name',
`email` varchar(255) NOT NULL COMMENT 'e-mail address',
`created_at` datetime,
`updated_at` datetime)
ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC
COMMENT='user table is the sample table'
;

-- +migrate Down
DROP TABLE IF EXISTS `user`;
//}

@<code>{sql-migrate up}コマンドを実行するとマイグレーションが実行されます。
マイグレーション実行後、@<code>{sql_sample}データベース上のテーブル情報を確認した結果が次の実行結果です@<fn>{bd_migrate_up}。

//cmd{
$ sql-migrate up
Applied 1 migration
$ mysql -h 127.0.0.1 --port ${SHOTEN6_MYSQL_PORT} -u${SHOTEN6_MYSQL_USER} \
     -D sql_sample -e "show tables;"
+----------------------+
| Tables_in_sql_sample |
+----------------------+
| gorp_migrations      |
| user                 |
+----------------------+
//}

@<code>{user}テーブルがマイグレーションによって作成されました。

//footnote[bd_migrate_up][@<code>{gorp_migrations}は@<code>{sql-migrate}が自動生成したマイグレーション情報を管理するためのテーブルです。]

== リポジトリパターンを使ってデータベース操作を実装する
GoからアクセスするMySQLサーバとテーブルの準備はできました。本節からは実際に実装コードを書いていきます。
今回の実装でクリーンアーキテクチャなどのレイヤードアーキテクチャで使うことを想定してリポジトリパターン@<fn>{repo_pattern}を用いて実装します。
//footnote[repo_pattern][@<href>{https://docs.microsoft.com/ja-jp/dotnet/standard/microservices-architecture/microservice-ddd-cqrs-patterns/infrastructure-persistence-layer-design#the-repository-pattern}]

=== リポジトリパターンを使った実装概要
リポジトリパターンはビジネスロジック、アプリケーションロジックからデータソースの永続化操作を分離する設計パターンです。リポジトリパターンと、クリーンアーキテクチャを代表とする種々のレイヤードアーキテクチャを組み合わせることで様々な恩恵を受けることができます。まず、データソースの永続化操作を特定のレイヤー（今回は@<code>{repository}パッケージ）にカプセル化することができます。その他のレイヤーはデータソースの永続化操作を考慮せず、ビジネスロジック・アプリケーションロジックのみを表現できます。また、依存性の注入@<fn>{wiki_di}（@<tti>{Dependency injection}、DI）でレイヤーを組み合わせることで、永続化操作の実装とビジネスロジック・アプリケーションロジックの依存関係を下げることができます。


//footnote[wiki_di][@<href>{https://ja.wikipedia.org/wiki/%E4%BE%9D%E5%AD%98%E6%80%A7%E3%81%AE%E6%B3%A8%E5%85%A5}]

Goの場合、レイヤーはパッケージ（ディレクトリ）構成で表現されることが多いです。今回は@<list>{bd_dirs}の構成で実装を行ないます。

//list[bd_dirs][パッケージ構成図]{
$ tree .
.
├── entity     // データモデルやビジネスロジックを格納するパッケージ
├── repository // データの永続化操作を行なうパッケージ
└── usecase    // アプリケーションロジックを格納するパッケージ
    └── port   // 抽象化したリポジトリ定義を置いておくサブパッケージ
//}

本来ならば、@<list>{bd_dirs}の他に@<code>{*sql.DB}オブジェクトを生成してデータベースへ接続する@<code>{gateways}パッケージが必要になるでしょう。しかし、今回は単純に@<code>{sql.Open}関数で@<code>{*sql.DB}オブジェクトを取得するだけの簡易実装を使うので省略します。また、Webサービス開発の場合はこの他にHTTPやgRPC@<fn>{grpc}の送受信を行なう@<code>{controllers}パッケージや@<code>{presenters}パッケージなどを作成されると思いますが、今回はデータベースの操作をする部分の実装に絞るため、割愛します。

//footnote[grpc][@<href>{https://grpc.io/}]

また、リポジトリのインターフェースについてはGo Code Review Comments@<fn>{wiki_crc}を愚直に守り、リポジトリの実装の利用者側である@<code>{usecase}パッケージのサブパッケージ（@<code>{port}パッケージ）に定義します。

//footnote[wiki_crc][@<href>{https://github.com/golang/go/wiki/CodeReviewComments#interfaces}]

=== リポジトリパターンの空実装を行なう
では、実際にリポジトリパターンを実装します。まずデータモデルである@<code>{entity.User}構造体を定義しておきます。
@<code>{entity.User}構造体は@<list>{bd_entity}の通りのフィールドを持ちます。

//list[entity_user][./entity/user.go]{
package entity

import (
  "time"
)

// User is sample structure.
type User struct {
  ID        int64     `db:"id"`
  Name      string    `db:"name"`
  Email     string    `db:"email"`
  CreatedAt time.Time `db:"created_at"`
  UpdatedAt time.Time `db:"updated_at"`
}
//}

構造体のフィールド名とデータベースのカラム名が異なる場合はタグを使ってマッピングを行ないます。
@<list>{entity_user}では@<code>{CreatedAt}フィールドを@<code>{created_at}カラムに対応付けるタグを定義しています。
簡略のために省いていますが、実践で定義するときは@<code>{ID}の@<code>{int64}型を独自型の@<code>{type UserID int64}にするなどをしたほうがベターでしょう。

次に@<code>{repository}パッケージの実装を行ないます。まずは簡単に @<code>{*sqlDB}オブジェクトのラッパーとなる @<code>{Repo}の定義（@<list>{bd_repodb}）と、 
@<code>{entity.User}を読み書きするメソッドの空定義（@<list>{bd_repo}）を用意します。


//list[bd_repodb][./repository/db.go]{
package repository

import (
  "database/sql"
)

// Repo *sql.DBをラップしたSQLデータベースへアクセスするためのリポジトリオブジェクト。
type Repo struct {
  db *sql.DB
}

// NewRepo 関数は依存性を注入した*Repoオブジェクトを生成するための初期化関数。
func NewRepo(db *sql.DB) *Repo {
  return &Repo{db}
}
//}

実際にアプリケーションロジックなどを作成し始めると、@<code>{Email}情報を使って@<code>{entity.User}を探したりするでしょうが、ひとまず読み書きするメソッドを一つずつ定義しました。

//list[bd_repo][./repository/user_repository.go]{
package repository

import (
  "context"

  "github.com/budougumi0617/go-sql-sample/entity"
)

// FindUser gets user from repository.
func (repo *Repo) FindUser(ctx context.Context, id int64) (*entity.User, error) {
  return nil, nil
}

// AddUser adds user to repository.
func (repo *Repo) AddUser(ctx context.Context, u *entity.User) error {
  return nil
}
//}


== github.com/DATA-DOG/go-sqlmockを使った単体テストの書き方
リポジトリの空実装を作成したあとはユニットテストを実装します。
まずデータベースを準備せずに実行できるテストを作成してみましょう。
正しく意図通りのSQLの発行がされたか確認できる@<code>{github.com/DATA-DOG/go-sqlmock}パッケージを利用します。

@<code>{go-sqlmock}パッケージは@<code>{*sql.DB}オブジェクトとして利用できるモックオブジェクトを生成します。テスト実装者は、モックオブジェクト生成時に同時に生成される@<code>{go-sqlmock.Sqlmock}オブジェクトを使って、モックオブジェクトへ期待するSQL文を設定することができます。
詳しい使い方はREADME@<fn>{sqlmock_rm}のサンプルコードや、GoDoc@<fn>{sqlmock_gd}を確認してください。

//footnote[sqlmock_rm][@<href>{https://github.com/DATA-DOG/go-sqlmock#tests-with-sqlmock}]
//footnote[sqlmock_gd][@<href>{https://godoc.org/github.com/DATA-DOG/go-sqlmock}]

@<code>{go-sqlmock}パッケージを利用したテストコードの抜粋が@<list>{bd_repotest}です@<fn>{bd_repotest}。
//footnote[bd_repotest][@<href>{https://github.com/budougumi0617/go-sql-sample/blob/master/repository/user_repository_test.go}]

//list[bd_repotest][./repository/user_repository_test.go]{
package repository

import (
  "context"
  "reflect"
  "testing"
  "time"

  sqlmock "github.com/DATA-DOG/go-sqlmock"

  "github.com/budougumi0617/go-sql-sample/entity"
)

// 検索結果として期待するUserデータ。
var o = entity.User{
  ID:        1,
  Name:      "budougumi0617",
  Email:     "budougumi0617@example.com",
  CreatedAt: time.Now(),
  UpdatedAt: time.Now(),
}

// SQLのSELECT文の結果として期待する行データ。
var existRows = func() *sqlmock.Rows {
  return sqlmock.NewRows([]string{
    "id", "name", "email",
    "created_at", "updated_at",
  }).AddRow(
    o.ID, o.Name, o.Email,
    o.CreatedAt, o.UpdatedAt,
  )
}

func TestRepo_FindUser(t *testing.T) {
  unknownID := int64(999)

  // *sql.DBモックを作成します。
  mockdb, mock, err := sqlmock.New()
  if err != nil {
    t.Fatal(err)
  }
  ctx := context.Background()
  // *sql.DBモックをDIしたリポジトリを生成します。
  repo := NewRepo(mockdb)

  // 最初に期待するUserデータを探すSELECT文の発行が行われることを想定します。
  // 期待するUserデータを返す挙動をモックに仕込みます
  mock.ExpectQuery(`
    SELECT id, name, email, created_at, updated_at FROM user WHERE id = \?
  `).WithArgs(o.ID).WillReturnRows(existRows())

  // 存在しないIDを使ったSELECT文の発行が行われることを想定します。
  // 空の行データを返す挙動をモックに仕込みます
  mock.ExpectQuery(`
    SELECT id, name, email, created_at, updated_at FROM user WHERE id = \?
  `).WithArgs(unknownID).WillReturnRows(sqlmock.NewRows([]string{}))

  got, err := repo.FindUser(ctx, o.ID)
  // 期待するUserデータが返ってきたかバリデーションを行なう

  got2, err := repo.FindUser(ctx, unknownID)
  // Userデータが返らなかったことのバリデーションを行なう

  // 期待したSQL文をモックが受け付けたか検証する
  if mock.ExpectationsWereMet() != nil {
    t.Errorf("mock has error %v", err)
  }
}
//}

@<list>{bd_repotest}のテストを実際に実行した結果が次になります。リポジトリはまだ空実装のため、期待したSQL文が発行されずテストが失敗しました。

//cmd{
$ go test ./repository
?     github.com/budougumi0617/go-sql-sample/entity  [no test files]
--- FAIL: TestRepo_FindUser (0.00s)
    user_repository_test.go:61:
    want {1 budougumi0617 budougumi0617@example.com
     2019-04-09 20:54:49.223196 +0900 JST m=+0.000430834
     2019-04-09 20:54:49.223196 +0900 JST m=+0.000431006
    }, but <nil>
    user_repository_test.go:72: mock has error <nil>
--- FAIL: TestRepo_AddUser (0.00s)
    user_repository_test.go:99:
    mock has error there is a remaining expectation which was not matched:
    ExpectedPrepare => expecting Prepare statement which:
          - matches sql: '
                INSERT INTO user \(name, email, created_at, updated_at\)
                VALUES \(\?, \?, \?, \?\)
            '
FAIL
FAIL  github.com/budougumi0617/go-sql-sample/repository  0.007s
# 紙面の都合上一部のテスト結果は改行を追加しています。
//}


=== database/sqlを使ったデータベース操作の実装
動作確認するテストコードも揃ったので、実装してみましょう。
Goにもデータベースへの処理を実装するときは@<code>{github.com/jinzhu/gorm}@<fn>{gorm}などの比較的リッチなフレームワークが存在します。
ただ、今回は基礎を押させるため、標準パッケージである@<code>{database/sql}パッケージを使ってデータベース処理を実装していきます。
コードを書く前に@<code>{database/sql}パッケージの内容を確認しておきましょう。

//footnote[gorm][@<href>{http://doc.gorm.io/}]

まずは公式ドキュメントを確認しておきます。GoDoc@<fn>{sql_doc}やGo WikiのSQLInterface@<fn>{sql_wiki}が参考になるでしょう。Goからデータベースに接続するためには接続先にあったドライバをblank importする必要があります。
MySQLの場合は、@<code>{github.com/go-sql-driver/mysql}@<fn>{bd_driver-mysql}がデファクトでしょう。

//footnote[bd_driver-mysql][@<href>{https://github.com/go-sql-driver/mysql}]


データベースの各ドライバは@<code>{init}関数という起動時に呼び出される特殊な関数を各パッケージ内に宣言しています@<fn>{mysql_drv}。

//list[mysql_init][github.com/go-sql-driver/mysqlのinit関数]{
func init() {
  sql.Register("mysql", &MySQLDriver{})
}
//}

@<code>{init}関数によって登録されたドライバを使って@<code>{sql.Open}関数を呼び出すことによってデータベース操作が始まります。

//list[init_open][blank importとデータベースの初期化]{
import (
  _ "github.com/go-sql-driver/mysql"
)

func foo() {
  db, err := sql.Open("mysql", "root:mysql@(localhost:3306)/database_name")

  // Do something...

}
//}

//footnote[sql_doc][@<href>{https://golang.org/pkg/database/sql/}]
//footnote[sql_wiki][@<href>{https://github.com/golang/go/wiki/SQLInterface}]
//footnote[mysql_drv][@<href>{https://github.com/go-sql-driver/mysql/blob/89ec2a9ec85e27afb0f0fcd3c2399e72cc360ae4/driver.go#L83-L85}]

==== sql.DB、sql.Conn、sql.Tx オブジェクト
@<code>{sql.Open}に成功すると、戻り値として@<code>{*sql.DB}オブジェクトが手に入ります。
@<code>{*sql.DB}オブジェクトにはいかにもなメソッド名の@<code>{QueryContext}メソッドが存在します。
しかし、@<code>{github.com/go-sql-driver/mysql}パッケージのGoDocをみると@<code>{QueryContext}メソッドは@<code>{*sql.Conn}オブジェクトや@<code>{*sql.Tx}オブジェクトにも存在します。どうやって使い分ければいいのでしょうか。

 * @<code>{ func (c *Conn) QueryContext(ctx context.Context, query string, args ...interface{\}) (*Rows, error)}
 * @<code>{func (db *DB) QueryContext(ctx context.Context, query string, args ...interface{\}) (*Rows, error)}
 * @<code>{func (tx *Tx) QueryContext(ctx context.Context, query string, args ...interface{\}) (*Rows, error)}


@<code>{*sql.DB}オブジェクトは複数のゴルーチンから利用されても安全に利用できることがGoDocに明記されています@<fn>{sql_db}。データベースへのコネクションプールと思えばよいようです。
また、一度 @<code>{sql.Open}するだけでよく何回も@<code>{Open}と @<code>{Close}するものでもないことがGoDocに明記されています@<fn>{sql_open}。

//footnote[sql_db][@<href>{https://golang.org/pkg/database/sql/#DB}]
//footnote[sql_open][@<href>{https://golang.org/pkg/database/sql/#Open}]

@<code>{*sql.DB}オブジェクトの@<code>{Conn}メソッドから取得できる@<code>{*sql.Conn}オブジェクトはデータベースとの一つのコネクションに相当します。
@<code>{*sql.Conn}オブジェクトは継続して同じコネクションでデータベース操作を行ないたいときに利用すべきでしょう。
たとえば、HTTPサーバのひとつのリクエスト中などは@<code>{*sql.Conn}オブジェクトで良さそうです。

@<code>{*sql.DB}オブジェクト、@<code>{*sql.Conn}オブジェクトの@<code>{BeginTx}メソッドから取得できる@<code>{*sql.Tx}オブジェクトはトランザクションを象徴するオブジェクトです。コミット、ロールバック操作を行ないたい時に利用すれば良さそうです。

@<code>{*sql.Conn}オブジェクトや@<code>{*sql.Tx}オブジェクトは利用後@<code>{Close}メソッドを呼ばないとコネクションを話さないので注意が必要です。

それでは、実際にSQLを発行するときのメソッドを簡単に紹介します。どれもプレースホルダーを利用することが可能です。
この他にも@<code>{Prepare}メソッドなども用意されているのですが、今回は入門編なので説明を省略します。

==== QueryContext/QueryRowContext メソッド
データベースに対する@<code>{SELECT}操作などは@<code>{QueryContext}メソッドを利用して実行することができます。

//list[query_context][QueryContextの利用例(GoDocのExampleより)]{
  age := 27
  rows, err := db.QueryContext(ctx, "SELECT name FROM users WHERE age=?", age)
  if err != nil {
    log.Fatal(err)
  }
  // 戻り値の *sql.Rowsオブジェクトも確実にCloseする必要があります。
  defer rows.Close()
  names := make([]string, 0)

  // 取得できた行データからstructや変数を書き換えていきます。
  for rows.Next() {
    var name string
    if err := rows.Scan(&name); err != nil {
      // Check for a scan error.
      // Query rows will be closed with defer.
      log.Fatal(err)
    }
    names = append(names, name)
  }
  // If the database is being written to ensure to check for Close
  // errors that may be returned from the driver. The query may
  // encounter an auto-commit error and be forced to rollback changes.
  rerr := rows.Close()
  if rerr != nil {
    log.Fatal(err)
  }

  // Rows.Err will report the last error encountered by Rows.Scan.
  if err := rows.Err(); err != nil {
    log.Fatal(err)
  }
//}

確実に一行以下の情報しか取得できないとわかっているときは@<code>{QueryRowContext}メソッドを使います。

//list[query_row_context][QueryRowContextの利用例(GoDocのExampleより)]{
  id := 123
  var username string
  var created time.Time
  err := db.QueryRowContext(
        ctx,
        "SELECT username, created_at FROM users WHERE id=?",
        id,
      ).Scan(&username, &created)
  switch {
  case err == sql.ErrNoRows:
    // 該当データがなかった場合はsql.ErrNoRowsがerrorとして返ってきます。
    log.Printf("no user with id %d\n", id)
  case err != nil:
    log.Fatalf("query error: %v\n", err)
  default:
    log.Printf("username is %q, account created on %s\n", username, created)
  }
//}

==== ExecContext メソッド
@<code>{INSERT}や@<code>{UPDATE}を実行したい場合は @<code>{ExecContext}メソッドを利用して実行することができます。

//list[exec_context][ExecContextの利用例(GoDocのExampleより)]{
  conn, err := db.Conn(ctx)
  if err != nil {
    log.Fatal(err)
  }
  defer conn.Close() // Return the connection to the pool.
  id := 41
  result, err := conn.ExecContext(
        ctx,
        `UPDATE balances SET balance = balance + 10 WHERE user_id = ?;`,
        id,
      )
  if err != nil {
    log.Fatal(err)
  }
  rows, err := result.RowsAffected()
  if err != nil {
    log.Fatal(err)
  }
//}

@<code>{INSERT}した場合は戻り値の@<code>{sql.Result}オブジェクトの@<code>{LastInsertId}メソッドから発行された行IDを取得することができます。


=== リポジトリの実装
では、実際に@<code>{database/sql}メソッドを利用してリポジトリのメソッドを実装します。

//list[bd_find_user][FindUserメソッドを実装する]{
func (repo *Repo) FindUser(ctx context.Context, id int64) (*entity.User, error) {
  // SELECTの結果の格納に利用するentity.User
  u := &entity.User{}
  conn, err := repo.db.Conn(ctx)
  if err != nil {
    return nil, err
  }
  defer conn.Close()

  err = conn.QueryRowContext(ctx, `
    SELECT id, name, email, created_at, updated_at FROM user WHERE id = ?
  `, id).Scan(
    &u.ID,
    &u.Name,
    &u.Email,
    &u.CreatedAt,
    &u.UpdatedAt,
  )
  switch {
  case err == sql.ErrNoRows:
    return nil, nil
  case err != nil:
    return nil, err
  }
  return u, nil
}
//}

@<code>{INSERT}を使って新規に@<code>{entity.User}を保存する実装は以下になりました。


//list[bd_add_user][AddUserメソッドを実装する]{
func (repo *Repo) AddUser(ctx context.Context, u *entity.User) error {
  conn, err := repo.db.Conn(ctx)
  if err != nil {
    return err
  }
  defer conn.Close()

  now := time.Now()
  res, err := conn.ExecContext(ctx, `
        INSERT INTO user (name, email, created_at, updated_at)
        VALUES (?, ?, ?, ?)
    `, u.Name, u.Email, now, now,
  )
  if err != nil {
    return err
  }

  id, err := res.LastInsertId() // 挿入した行のIDを取得
  if err != nil {
    return err
  }
  u.ID = id
  u.CreatedAt = now
  u.UpdatedAt = now

  return nil
}
//}

リポジトリの実装後に改めてテストを実行してみます。

//cmd{
$ go test -v ./repository
=== RUN   TestRepo_FindUser
--- PASS: TestRepo_FindUser (0.00s)
=== RUN   TestRepo_AddUser
--- PASS: TestRepo_AddUser (0.00s)
PASS
ok    github.com/budougumi0617/go-sql-sample/repository  0.009s
//}

今度は期待した通りのSQLが発行されていることが確かめられました@<fn>{but_mock}。

//footnote[but_mock][ただ、これはモックを使ったテストです。モックが受け付けたSQL文とモックが返す結果が実動作を正しく表現できているかはテスト実装者のスキルに依存します。]

== UseCaseの実装
リポジトリパターンの恩恵を知るために、@<code>{reposiotory}パッケージを利用する簡単な@<code>{usecase}パッケージを作成してみます。
通常のレイヤードアーキテクチャであれば、リポジトリから取得したデータモデルを使ってビジネスロジックを呼び出したり、アプリケーションロジックの実装を行ないます。今回はサンプルなので、リポジトリからの取得したデータモデルをそのまま結果を返したり、ただデータモデルを生成するだけの薄い実装にします。

まずはリポジトリを@<code>{usecase}パッケージで扱うためのインターフェースを@<code>{usecase/port}パッケージに定義しておきます@<fn>{bd_port}。

//footnote[bd_port][@<href>{https://github.com/budougumi0617/go-sql-sample/blob/master/usecase/port/user_accessor.go}]

//list[bd_port][./usecase/port/user_accessor.go]{
package port

import (
  "context"

  "github.com/budougumi0617/go-sql-sample/entity"
)

// UserAccessor is a set of reader and writer for User in a data store.
type UserAccessor interface {
  UserReader
  UserWriter
}

// UserReader retrieves User data from a data store.
type UserReader interface {
  FindUser(context.Context, int64) (*entity.User, error)
}

// UserWriter stores User data into a data store.
type UserWriter interface {
  AddUser(context.Context, *entity.User) error
}
//}

@<code>{usecase}パッケージの実装が@<list>{bd_usecase}です@<fn>{bd_usecase}。

//footnote[bd_usecase][@<href>{https://github.com/budougumi0617/go-sql-sample/blob/master/usecase/user_usecase.go}]

//list[bd_usecase][./usecase/user_usecase.go]{
package usecase

import (
  "context"

  "github.com/budougumi0617/go-sql-sample/entity"
  "github.com/budougumi0617/go-sql-sample/usecase/port"
)

// UserCase entitu.Userを操作するリポジトリを内包するユースケース
type UserCase struct {
  ua port.UserAccessor
}

// NewUserCase リポジトリをDIする初期化関数。
func NewUserCase(ua port.UserAccessor) *UserCase {
  return &UserCase{
    ua: ua,
  }
}

// Save リポジトリを使ってentity.Userを永続化するユースケース。
func (au *UserCase) Save(ctx context.Context, name, email string) (int64, error) {
  u := &entity.User{
    Name:  name,
    Email: email,
  }
  err := au.ua.AddUser(ctx, u)
  if err != nil {
    return 0, err
  }

  return u.ID, nil
}

// Find リポジトリを使ってentity.Userを探すユースケース。
func (au *UserCase) Find(ctx context.Context, id int64) (*entity.User, error) {
  u, err := au.ua.FindUser(ctx, id)
  if err != nil {
    return nil, err
  }
  return u, nil
}
//}

@<code>{usecase.UserCase}は@<code>{usecase/port}のインターフェースを介してリポジトリを操作します@<fn>{about_interface}。
外部からDIされるその具象型が @<code>{go-sql-sample/repository.Repo}である必要はありません。
そのため、@<code>{usercase.UserCase}のメソッドをテストする際は、SQLのことを考えずにテストすることが出来ます。
たとえば、@<list>{simple_repo}のような簡易リポジトリをDIすることも可能です。

//footnote[about_interface][本来であれば @<code>{entity.User}に対するユースケースの操作の責務に応じてインターフェースを使いわけるところです。たとえば、読み込みしか行わないならば、@<code>{port.UserReader}インタフェースを使います。ただ今回のサンプル実装は読み書き両方行っているので、@<code>{port.UserAccessor}を使っています。]

//list[simple_repo][簡易リポジトリを使ってUserCaseオブジェクトを準備する例]{
type FakeRepo struct{}

// AddUser はport.UserAccessorを満たすための実装
func (r *Repo) AddUser(ctx context.Context, u *entity.User) error {
  u.ID = 100
  return nil
}

// FindUser はport.UserAccessorを満たすための実装
func (r *Repo) FindUser(ctx context.Context, id int64) (*entity.User, error) {
  return &entity.User{}, nil
}

func TestUserCase(t *testing.T) {
  uc := NewUserCase(&FakeRepo{})
  // ...
}
//}

このようにデータベースを利用せずにテストできる@<code>{usecase}パッケージの内容は本章の主旨と合わないため割愛します。

== MySQLサーバと接続するテスト
実際にMySQLサーバ上のデータベースを利用したテストコードを書いてみましょう。
本来ならば、テーブルの初期化やクリーンアップも行なう必要がありますが、
今回はまずユーザーを追加するだけの簡単なテストのみ書いてみます。

//list[e2e][./e2e/usecase_test.go]{
// +build e2e

package e2e

import (
  "context"
  "database/sql"
  "fmt"
  "log"
  "os"
  "testing"

  "github.com/budougumi0617/go-sql-sample/repository"
  "github.com/budougumi0617/go-sql-sample/usecase"
  _ "github.com/go-sql-driver/mysql"
)

func TestUserCase_Save(t *testing.T) {
  okName := "budougumi0617"
  okEmail := "budougumi0617@example.com"
  type args struct {
    name, email string
  }
  okArgs := args{
    name:  okName,
    email: okEmail,
  }
  tests := []struct {
    name string
    args args
  }{
    {
      name: "Correct",
      args: okArgs,
    },
  }
  for _, tt := range tests {
    t.Run(tt.name, func(t *testing.T) {
      u := os.Getenv("SHOTEN6_MYSQL_USER")
      p := os.Getenv("SHOTEN6_MYSQL_PORT")
      db, err := sql.Open(
        "mysql",
        fmt.Sprintf(
          "%s:@(localhost:%s)/sql_sample?parseTime=true&loc=Asia%%2FTokyo",
          u, p,
        ),
      )
      if err != nil {
        log.Fatalln(err)
      }
      ctx := context.Background()
      repo := repository.NewRepo(db)
      uc := usecase.NewUserCase(repo)

      got, err := uc.Save(ctx, tt.args.name, tt.args.email)

      if err != nil {
        t.Errorf("want no err, but has error %#v", err)
      }

      if got == 0 {
        t.Error("ID was 0")
      }
    })
  }
}
//}

Build Constraints@<fn>{bconst}をつけておくことで、@<code>{go test}コマンドを実行したときもビルドタグがない場合はテストが動作しないようにできます。
//footnote[bconst][@<href>{https://golang.org/pkg/go/build/#hdr-Build_Constraints}]

@<code>{go test}コマンドでテストを実行してみましょう@<fn>{bd_test_count}。
明示的に除外しなくても、@<code>{e2e}パッケージのテストコードが実行されないのがわかります。

//footnote[bd_test_count][@<code>{-count=1}はテストがキャッシュされないように指定しています。]

//cmd{
$ go test -v -count=1 ./...
?     github.com/budougumi0617/go-sql-sample/entity  [no test files]
=== RUN   TestRepo_FindUser
--- PASS: TestRepo_FindUser (0.00s)
=== RUN   TestRepo_AddUser
--- PASS: TestRepo_AddUser (0.00s)
PASS
ok    github.com/budougumi0617/go-sql-sample/repository  0.006s
?     github.com/budougumi0617/go-sql-sample/usecase  [no test files]
?     github.com/budougumi0617/go-sql-sample/usecase/port  [no test files]
//}

ビルドタグを付けて@<code>{go test}コマンドを実行すると、@<code>{e2e} パッケージのテストも実行されていることがわかります。

//cmd{
$ go test -v -count=1 -tags e2e ./...
=== RUN   TestUserCase_Save
=== RUN   TestUserCase_Save/Correct
--- PASS: TestUserCase_Save (0.01s)
    --- PASS: TestUserCase_Save/Correct (0.01s)
PASS
ok    github.com/budougumi0617/go-sql-sample/e2e  0.027s
?     github.com/budougumi0617/go-sql-sample/entity  [no test files]
=== RUN   TestRepo_FindUser
--- PASS: TestRepo_FindUser (0.00s)
=== RUN   TestRepo_AddUser
--- PASS: TestRepo_AddUser (0.00s)
PASS
ok    github.com/budougumi0617/go-sql-sample/repository  0.009s
?     github.com/budougumi0617/go-sql-sample/usecase  [no test files]
?     github.com/budougumi0617/go-sql-sample/usecase/port  [no test files]
//}

ここまででMySQLサーバを利用しないテスト、MySQLサーバを利用するテストを準備することができました。

最後に、次節でGitHubにコードがプッシュするたびに自動テストを行なうための設定をしたいと思います。
その前にここまでで行なったマイグレーションなどのコマンドライン操作を@<code>{Makefile}にしておきます。
@<code>{Makefile}の内容は@<list>{makefile}の通りです@<fn>{makefile}。

//footnote[makefile][@<href>{https://github.com/budougumi0617/go-sql-sample/blob/master/Makefile}]

//list[makefile][./Makefile]{    
.PHONY: create up e2e mysql.start mysql.stop

create:
  mysql -h 127.0.0.1 --port ${SHOTEN6_MYSQL_PORT} \
      -u${SHOTEN6_MYSQL_USER} < db/database.sql

up:
  sql-migrate up

e2e:
  go test -v -tags e2e ./...

mysql.start:
  docker run --rm -d -e MYSQL_ALLOW_EMPTY_PASSWORD=yes \
    -p $(SHOTEN6_MYSQL_PORT):3306 --name mysql_tmp mysql:5.7

mysql.stop:
  docker stop mysql_tmp
//}


== CircleCI上でMySQLサーバを利用する自動テストを実施する
業務で取り組む場合、@<code>{master}ブランチへマージするタイミングや、Pull Requestを作成・更新するたびに自動テストを実施して品質を担保します。
このような継続的インテグレーション@<fn>{bd617_ci}（CI、@<tti>{Continuous Integration})の仕組みをデータベースを利用した実装でも行なってみましょう。
今回はCircleCIというクラウドサービスを使って自動テストを行ないます。CircleCIはプログラムの開発を自動化するためのクラウドサービスです。
CircleCIは利用もたやすく、publicなリポジトリで利用する分には無料です。

//footnote[bd617_ci][@<href>{https://aws.amazon.com/jp/devops/continuous-integration/}]


CircleCIの設定ファイルは@<code>{./.circleci/config.yml}という名前で保存します。@<list>{bd_ci_config}が今回用意したCircleCI用の設定です@<fn>{bd_short_setting}。

//footnote[bd_short_setting][主題と関係ない設定は削除して載せています。]

//list[bd_ci_config][./.circleci/config.yml]{
version: 2.1
executors:
  default:
    docker:
      # CircleCI公式のGoを実行するためのイメージ
      # 最初に指定したimageがprimary imageとして各ジョブが実行されるコンテナになる
      - image: circleci/golang:1.12.2
      # CircleCI公式のMySQLイメージ
      # 各ステージからはlocalhost:3306に接続するとMySQLサーバにアクセスできる
      - image: circleci/mysql:5.7-ram

    # ジョブが実行されるディレクトリ
    working_directory: /go/src/github.com/budougumi0617/caww

    # 各コンテナに設定される環境変数
    environment:
      - GO111MODULE: "on"
      - MYSQL_ALLOW_EMPTY_PASSWORD: yes
      - MYSQL_ROOT_PASSWORD: ''
      - MYSQL_DATABASE: sql_sample
      - SHOTEN6_MYSQL_USER: root
      - SHOTEN6_MYSQL_PORT: 3306

jobs:
  # e2eという名前でジョブを宣言する（本来は複数のジョブを宣言できる）
  e2e:
    executor:
      # 上記で宣言したGo/MySQLの設定でジョブを実行する
      name: default
    steps:
      - checkout
      - run: mkdir -p $TEST_RESULTS
      - run:
          name: Install MySQL client
          command: sudo apt install -y mysql-client
      - run:
          name: Vendoring
          command: go mod download
      # マイグレーションツールのインストール
      - run:
          name: Install migration tool
          command: go get github.com/rubenv/sql-migrate/sql-migrate
      - run:
          name: Wait for DB
          # CircleCIの公式イメージは予めdockerizeコマンドがインストールされている
          # MySQLサーバと通信が出来るまで待機する
          command: dockerize -wait tcp://127.0.0.1:3306 -timeout 120s
      - run:
          name: Create database
          command: make init
      - run:
          name: Execute migration
          command: make up
      - run:
          name: Run all unit tests
          command: make e2e

workflows:
  build-and-test:
    jobs:
      - e2e
//}

各設定の詳細はCircleCIの公式ページ@<fn>{cci_cfg}を参照してください。
大事な部分は@<code>{image}部分でMySQLコンテナを起動していること、
@<code>{dockerize}コマンドでMySQLのサービスが起動するのを待機するところです。


//footnote[cci_cfg][@<href>{https://circleci.com/docs/2.0/config-intro/}]

設定ファイルを追加できたら、GitHub（やCircleCIがアクセスできるソースコードホスティングサイト）にソースコードを公開してみましょう。
事前にGitHub上に空のリモートリポジトリを作成しておき、今回作成した@<code>{go-sql-sample}内のコードをリモートリポジトリへプッシュします。

//cmd{
$ git init
$ git add --all
$ git commit -m "Published"
$ git remote origin git@github.com:budougumi0617/go-sql-sample.git
$ git push -f
//}

リモートリポジトリにソースコードを公開したあとはCircleCI上でビルドする設定を行ないます。CircleCIのダッシュボードに@<fn>{cci_board}にアクセスし、左側のメニューから"ADD PROJECTS@<fn>{cci_addproj}をクリックします。
画面遷移が終わると自分が公開しているリポジトリの一覧が表示されるので、本章で作成したコードがプッシュされているリポジトリの"Set Up Project"をクリックします。
"Set Up Project"をクリックしたあとは、言語などを選びビルドの初期設定を行なう画面に遷移しますが、@<code>{.circleci/config.yml}を作成してプッシュしているので作業は必要ありません。
ページ途中にある"Start Building"ボタンをクリックしてビルドをしてみましょう。
しばらく立つと@<code>{.circleci/config.yml}の手順に則って自動テストが始まります。
実際にCircleCIで自動テストを実行した結果が@<img>{bd_circleci_result}@<fn>{cci_result}です。


//image[bd_circleci_result][CircleCI上でMySQLサーバを利用したテストを実行した結果][scale=0.8]{
//}

あとはコミットやPRが作成されるたびに自動テストが実行されます。
これでCircleCI上でMySQLを利用したテストが実行できるようになりました。

//footnote[cci_board][@<href>{https://circleci.com/dashboard}]
//footnote[cci_addproj][@<href>{https://circleci.com/add-projects/gh/YOUR_GITHUB_ACCOUNT}]
//footnote[cci_result][@<href>{https://circleci.com/gh/budougumi0617/go-sql-sample/3}]

== おわりに
本章ではデータベースを利用したGoの実装について紹介しました。
単にデータベースを操作するコードを書くだけではなく、次の箇条書きにまとめた点についても紹介しました。

 * Dockerを利用したデータベースの準備方法
 * @<code>{github.com/rubenv/sql-migrate}を使ったデータベースのマイグレーション方法
 * リポジトリパターンを使ったデータベースアクセスと業務ロジックを疎にする設計方針
 * @<code>{github.com/DATA-DOG/go-sqlmock}を使った単体テストの書き方
 * @<code>{database/sql}パッケージを使ったデータベース操作の入門
 * MySQLサーバを利用した結合テストの書き方
 * CircleCI上でMySQLサーバを利用した自動テストを行なう設定例


@<code>{sqlmock}などを使ったテストはホワイトボックス過ぎて敬遠される方もいらっしゃると思います。
が、今回は”こういうライブラリがあって、こんな実装アプローチができる”ということを共有する意味もこめて敢えて利用してみました。
実際の業務で利用するためにはまだまだ不足している内容も多いですが、本章がみなさんの学習の手助けになれば幸いです。

