= sql.DB実践入門

== はじめに

freee株式会社 @<fn>{bd617_freee} のバックエンドエンジニアの@budougumi0617@<fn>{bd617_twitter}です。 
freee株式会社でバックエンドエンジニアをしている@<tt>{@budougumi0617}@<fn>{bd617_twitter}です。

//footnote[bd617_twitter][@<href>{https://twitter.com/budougumi0617}]

=== 本章の目的・ゴール
本章では、Goから @<tt>{RDBMS}@<fn>{bd617_rdbms}（関係データベース管理システム、 @<tti>{Relational Database Management System}）を操作する実装やテストの作成方法を紹介します。なお、簡略化のため、本章では以降@<tt>{hRDBMS}のことを単にデータベースと記載します。


2019年になり、Goの日本語書籍は多く発売されています。また、インターネット上の日本語記事も多くなりました。しかし、データベースを使った実装、テストの方法の基本的な部分をまとめた入門用の文章はほとんどないかと思います。そこで本章では今までGoのコードを使ってデータベースを操作する実装・テストを行なったことがない方向けに、次の箇条書きの情報をまとめました@<fn>{bd_versions_note}。

 * データベースの準備方法
 * @<tt>{github.com/rubenv/sql-migrate}@<fn>{bd_sql-migrate} を使ったデータベースのマイグレーション方法
 * @<tt>{github.com/go-sql-driver/mysql}@<fn>{bd_driver-mysql}を使った基本的なデータベース操作の実装
 * @<tt>{github.com/DATA-DOG/go-sqlmock}@<fn>{bd_sql-mock}を使った単体テストの書き方
 * データベースを利用した結合テストの書き方
 * 継続的インテグレーションをするためのCircleCI@<fn>{bd_circleci}の設定方法

なお、今回はデータベースとして一般的なMySQL@<fn>{bd617_mysql}を使います。
本章で利用する各種技術のバージョンは@<table>{bd_versions}の通りです。

//table[bd_versions][各種バージョン]{
対象技術	バージョン	補足
----------------------------
Go	1.12.2 darwin/amd64
MySQL	5.7
CircleCI configuration	2.1
github.com/rubenv/sql-migrate	v1.0.0
github.com/go-sql-driver/mysql	v1.4.1
github.com/DATA-DOG/go-sqlmock	v1.3.3
//}

また、Goは環境変数として@<tt>{GO111MODULE=on}を設定して、Moudleモード@<fn>{bd_modules}を有効にしています。

//footnote[bd617_rdbms][@<href>{https://en.wikipedia.org/wiki/Relational_database_management_system}]
//footnote[bd617_mysql][@<href>{https://www.mysql.com/}]
//footnote[bd_versions_note][各技術・用語については以降の章で実際に利用する時に説明します。]
//footnote[bd_sql-migrate][@<href>{https://github.com/rubenv/sql-migrate}]
//footnote[bd_driver-mysql][@<href>{https://github.com/go-sql-driver/mysql}]
//footnote[bd_sql-mock][@<href>{https://github.com/DATA-DOG/go-sqlmock}]
//footnote[bd_circleci][@<href>{https://circleci.com/}]
//footnote[bd_modules][@<href>{https://github.com/golang/go/wiki/Modules}]

=== 事前準備

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
