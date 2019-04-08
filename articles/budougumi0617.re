= sql.DB実践チュートリアル

== はじめに

freee株式会社でバックエンドエンジニアをしている@<tt>{@budougumi0617}@<fn>{bd617_twitter}です。

//footnote[bd617_twitter][@<href>{https://twitter.com/budougumi0617}]

=== 本章の目的・ゴール
本章では、GoからRDBMS@<fn>{bd617_rdbms}（関係データベース管理システム、 @<tti>{Relational Database Management System}）を操作する実装やテストの作成方法を紹介します。なお、簡略化のため、本章では以降RDBMSのことを単にデータベースと記載します。


2019年になり、Goの日本語書籍は多く発売されています。また、インターネット上の日本語記事も多くなりました。しかし、データベースを使った実装、テストの方法の基本的な部分をまとめた入門用の文章はほとんどないかと思います。そこで本章では今までGoのコードを使ってデータベースを操作する実装・テストを行なったことがない方向けに、次の箇条書きの情報をまとめました@<fn>{bd_versions_note}。

 * Docker@<fn>{bd_docker}を利用したデータベースの準備方法
 * @<tt>{github.com/rubenv/sql-migrate}@<fn>{bd_sql-migrate}を使ったデータベースのマイグレーション方法
 * リポジトリパターンを使ったデーベースと疎にする設計方針
 * @<tt>{github.com/go-sql-driver/mysql}@<fn>{bd_driver-mysql}を使った基本的なデータベース操作の実装
 * @<tt>{github.com/DATA-DOG/go-sqlmock}@<fn>{bd_sql-mock}を使った単体テストの書き方
 * データベースを利用した結合テストの書き方
 * 継続的インテグレーションをするためのCircleCI@<fn>{bd_circleci}の設定方法

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

また、Goは環境変数として@<tt>{GO111MODULE=on}を設定して、Moudleモード@<fn>{bd_modules}を有効にしています。
なお、以降のコマンドライン操作はすべてMacOS上でのみ動作確認をしています。WindowsやLinux環境をご利用の方は適宜コマンド操作を置き換えてください。

//footnote[bd617_rdbms][@<href>{https://en.wikipedia.org/wiki/Relational_database_management_system}]
//footnote[bd_versions_note][各技術・用語については以降の章で実際に利用する時に説明します。]
//footnote[bd_docker][@<href>{https://www.docker.com/}]
//footnote[bd_sql-migrate][@<href>{https://github.com/rubenv/sql-migrate}]
//footnote[bd_driver-mysql][@<href>{https://github.com/go-sql-driver/mysql}]
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

本章の以降の操作は、特に明記がない限りここで作成した@<tt>{go-sql-sample}ディレクトリ内で行われます@<fn>{bd_gopath}。

//footnote[bd_gopath][Moduleモードを利用している場合は@<tt>{$GOPATH}以下にプロジェクトを作成する必要はありませんが、@<tt>{$GOPATH}以下の作成が禁止されたわけではありません。]

次に、環境変数として@<list>{bd_env}の環境変数を定義しておきます。

//list[bd_env][環境変数の設定]{
export GO111MODULE=on
export SHOTEN6_MYSQL_USER=root
export SHOTEN6_MYSQL_PORT=3306
//}

もし既にMySQLをローカルで起動していて、次節の方法でMySQLコンテナを利用する場合は、既存のMySQLとポートが競合してしまうので、@<tt>{SHOTEN6_MYSQL_PORT}の値を別の値に変更してください（例：@<tt>{43306}など）。

なお、必須ではありませんが、私は@<tt>{go-sql-sample}ディレクトリに@<tt>{.envrc}として@<list>{bd_env}の環境変数を保存しました。ファイルに保存した状態で@<tt>{github.com/direnv/direnv}というツールを使うと、そのディレクトリでコマンドを実行するときだけ自動で環境変数が定義されるので便利です。次のコマンドラインの実行例は@<tt>{direnv}を利用している状態で各ディレクトリで環境変数を確認した実行結果です。

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

ローカルにMySQLのクライアントツールがインストールされていない場合は、@<tt>{mysql}コマンドもインストールしておきましょう。

//cmd{
$ brew install mysql
//}

また、MacでDockerを使う場合はDocker for Macが便利です。インストールページ@<fn>{bd_docker_mac}の手順に則ってDockerを利用できるようにしておきましょう。

//footnote[bd_docker_mac][@<href>{https://hub.docker.com/editions/community/docker-ce-desktop-mac}]

==={bd_hd_docker} Dockerを利用したデータベース環境の準備方法
環境変数の設定準備が完了したら、Goから接続するデータベースとしてMySQLをローカルで起動しておきます。MySQLはSQL@<fn>{bd_sql}を扱うオープンソースで公開されているデータベースのひとつです。
ここでは、Dockerを使ってコンテナとしてMySQLサーバを起動します。Dockerコンテナとしてデータベースを起動することで、利用が終了したあと簡単に廃棄することができます。既にMySQLのデータベース環境をローカルに構築済みの方はその環境を利用してもかまいません。

//footnote[bd_sql][@<href>{https://ja.wikipedia.org/wiki/SQL}]

Dockerを利用してMySQLコンテナを起動するには次のコマンドを実行するだけで十分です。ローカルにDockerイメージがない場合は自動的にイメージのダウンロードが行われます。
コマンドオプションはデーモンとして起動、空パスワードの許可、ポートを環境変数@<tt>{SHOTEN6_MYSQL_PORT}として設定し、@<tt>{mysql_tmp}という名前で起動させるという意味です。

//cmd{
$ docker run --rm -d -e MYSQL_ALLOW_EMPTY_PASSWORD=yes \
    -p $(SHOTEN6_MYSQL_PORT):3306 --name mysql_tmp mysql:5.7
//}

@<tt>{docker run}コマンドでMySQLのコンテナを起動したあとは、@<tt>{mysql}コマンドを使って動作確認をしておきましょう。
なお、コンテナ起動直後の数秒間はMySQLの初期化が行われているので、少し時間を置いてから次の`mysql`コマンドを実行してください。
デフォルトで作成されている管理用の@<tt>{database}の情報が取得できたら接続成功です。

//cmd{
$ mysql -h 127.0.0.1 --port ${SHOTEN6_MYSQL_PORT} -uroot -e "show databases;"
+--------------------+
| Database           |
+--------------------+
| information_schema |
| caww               |
| mysql              |
| performance_schema |
| sys                |
+--------------------+
//}

なお、利用後は以下のコマンドで停止させることができます。

//cmd{
$ docker stop mysql_tmp
//}

== github.com/rubenv/sql-migrateを使ったデータベースのマイグレーション方法
== github.com/go-sql-driver/mysqlを使った基本的なデータベース操作の実装

 * github.com/DATA-DOG/go-sqlmockを使った単体テストの書き方
 * データベースを利用した結合テストの書き方
 * 継続的インテグレーションをするためのCircleCI@<fn>{bd_circleci}の設定方法

== データベース環境の準備

== リポジトリーパターンの実装
=== クリーンアーキテクチャとリポジトリパターン
=== データベースの作成
=== マイグレーションツールの導入
=== MySQLクライアントでアクセスしてみる
== database/sqlの基本
=== sql.DB、sql.Conn、sql.Txの使いわけ
@<code>{sql.DB}などがあります。
@<tt>{sql.DB}などがあります。

=== QueryContext
=== Prepareと
ExecContextとPrepare/Execはどちらがいいんだろうか？


== データベースを扱う単体テストを書く
== MySQLを利用するテストを書く

//cmd{
$ ls
//}
== テストをCircleCIで実行する


== おわりに


//list[tutorial_main][go-cloud/samples/tutorial/main.goから抜粋したファイルアップロード処理]{
ctx := context.Background()
// Open a connection to the bucket.
b, err := setupBucket(context.Background(), *cloud, *bucketName)

// Prepare the file for upload.
data, _ := ioutil.ReadFile(file)

w, _ := b.NewWriter(ctx, file, nil)
w.Write(data)
w.Close()
//}
