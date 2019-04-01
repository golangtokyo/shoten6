= ユニットテストの考え方と実現するためのGoのテスト技法

== はじめに

はじめまして、hgsgtkです。業務ではサーバーサイドエンジニアとして、GoやPHPをメインに使ってアプリケーションの開発をしています。この章では、ユニットテストについて扱います。テストを書くことは、業務でのアプリケーション開発にとって重要な要素のひとつです。ユニットテストの書き方についての記事は多数ありますが、「なぜその方法が良いのか」という考え方についての理解の場は少ないです。そのため、本章では、「どのようなテストが効果的か」という点とそれをどう実現するかについて説明します。この章を読むことで、普段何気なく書くテストについて立ち止まって考える機会になることを期待しています。

== Goのテストの基本
Goでは、 @<code>{go test} というサブコマンドが用意されています。これは、ある種の規約に従って構成されるGoパッケージに対するテストドライバです。

@<code>{_test.go} で終わるファイルは、@<code>{go build} によってビルドした場合、ビルド対象となりません。一方、 @<code>{go test} でビルドされた場合には対象パッケージの一部としてビルドされます。

テストを作成するには、 @<code>{testing} パッケージをインポートしたファイルで次のシグネチャを持った関数を作成します。

//source[sample_test.go]{
  func TestName(t *testing.T) {
    // ...
  }
//}

テスト関数名は、 @<code>{Test} で始め、以降の接頭辞 @<code>{Name} は大文字で始める必要があります。

#@# TODO: 例えば、こういう関数ならこういうテストを書くのっていうのを書く


== ユニットテストの考え方
ユニットテストにおいて意識すべき考え方として、「費用対効果」という視点があります。
ユニットテストを書いている方であれば何かしらのメリットを感じて実践していることでしょうが、ユニットテストによって得られるメリットとしていくつかの「節約コスト」があります。一般的には次のような節約コストが挙げられます。

* 手動ユニットテストのコスト
** プログラムで実行可能な自動ユニットテストが準備されることで、手動でのユニットテストから開放されます。
* 欠陥の早期発見による修正コスト
** ユニットテストにより早期に欠陥を発見・修正することで、以降のテストフェーズやリリース後の欠陥発生による修正コストを削減することができます、
* ドキュメンテーションコスト
** ユニットテストを書くことで、テスト作成者は対象の仕様・期待した動作をテストで表現することができます。
* デバッグコスト
** ユニットテストは簡単に実行可能なため、対象のデバッグの難易度は下がります。

一方で、ユニットテストを維持する上でコストがかかることも事実です。一般的には次のようなコストがかかるとされます。

* ユニットテストの学習コスト
** ユニットテスト自体の書き方を覚えるコストは少なからず発生します。
* 新規テスト作成コスト
** 新規実装においてテストを書く場合、テストを作成する工数が発生します。
* 既存テスト維持コスト
** 既存の実装を修正した際、場合によっては既存テストの修正が必要になります。
* テスト実行時間中のコスト
** プログラマはテストが通ることを確認するため、テスト実行時間において待つ工数が発生します。

ユニットテストを効果的なものにするためには、ユニットテストによる節約コストをユニットテスト自体のコストより高い状態を目指す必要があります。つまり、かかるコストに対して得られる利益が大きい「費用対効果の高い」テストを意識して取り組むということです。

== t.Error・t.Fatalの使い分け

== テーブル駆動テスト・サブテスト

== インターフェース・スタブ実装

== テストパッケージ


== おわりに
