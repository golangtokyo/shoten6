= 費用対効果の高いユニットテストを実現するためのGoのテスト基礎

== はじめに

はじめまして、hgsgtk@<fn>{hgsgtk}です。業務ではサーバーサイドエンジニアとして、GoやPHPをメインに使ってアプリケーションの開発をしています。

//footnote[hgsgtk][@<href>{https://twitter.com/hgsgtk}]

この章では、ユニットテストについて扱います。ユニットテストを書くことは、業務でのアプリケーション開発にとって重要な要素のひとつです。ユニットテストの書き方についての記事は多数ありますが、「なぜその方法が良いのか」という考え方についての理解の場は少ないです。そのため、本章では、「どのようなテストが効果的か」という点とそれをどう実現するかについて説明します。この章を読むことで、普段何気なく書くテストについて立ち止まって考える機会になることを期待しています。

== Goのユニットテストの基本
まず、Goのユニットテスト作成の基本を抑えましょう。Goでは、 単体テストを行うためのコマンドとして、@<code>{go test} というサブコマンドが用意されています。@<code>{_test.go}というサフィックスの付いたファイルを対象にしてテストを実行します。

//list[goTestExecution][go testによるテスト実行][]{
% go test github.com/hgsgtk/go-snippets/testing-codes/fizzbuzz
ok  	fizzbuzz	0.006s
//}

@<code>{_test.go} で終わるファイルは、@<code>{go build} によってビルドした場合、ビルド対象となりません。一方、 @<code>{go test} でビルドされた場合には対象パッケージの一部としてビルドされます。

テストを作成するには、 testing パッケージをインポートしたファイルで次のシグネチャを持った関数を作成します。

//list[sampleTestGo][sample_test.go][go]{
package sample_test

import "testing"

func TestName(t *testing.T) {
	// ...
}
//}

テスト関数名は、 @<code>{Test} で始め、以降の接頭辞 @<code>{Name} は大文字で始める必要があります。また、引数定義には、@<code>{*testing.T}を設定します。

ひとつサンプルを見てましょう。"hello" と返却する@<code>{SayHello()}という関数があるとします。
//list[sampleHello][sample.go][go]{
package sample

func SayHello() string {
	return "hello"
}
//}

この場合、次のようなテストコードを作成することができます。

//list[sampleTestHello][sample_test.go][go]{
func TestSayHello(t *testing.T) {
	want := "hello"
  // SayHelloの戻り値が期待値とことなる場合エラーとして処理する
	if got := sample.SayHello(); got != want {
		t.Errorf("SayHello() = %#v, want %#v", got, want)
	}
}
//}

作成したテストを先程紹介した @<code>{go test} で実行するとテストが通ることが確認できます。

//list[sampleTestExecution][作成したSayHelloに対するテストが通る][]{
-> % go test -v github.com/hgsgtk/go-snippets/testing-codes/sample
=== RUN   TestSayHello
--- PASS: TestSayHello (0.00s)
PASS
ok  	github.com/hgsgtk/go-snippets/testing-codes/sample	0.007s
//}

このテストが失敗する場合は次のような出力結果が得られます。

//list[sampleTestExecutionFailed][作成したSayHelloに対するテストが失敗する][]{
-> % go test -v github.com/hgsgtk/go-snippets/testing-codes/sample
--- FAIL: TestSayHello (0.00s)
  sample_test.go:16: SayHello() = "hellox", want "hello"
FAIL
FAIL	github.com/hgsgtk/go-snippets/testing-codes/sample	0.008s
//}

ここまでが、Goのユニットテストを作る上での基礎知識になります。基本的にはこれまで紹介した知識で最低限のテストコードを作成できるようになります。

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

#@# xUTP

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

== 適切なエラーハンドリング
Goの特徴的な言語仕様として、 アサート（assertion） が提供されていない点が挙げられます。これは、@<href>{https://golang.org/doc/faq#assertions}にて理由が説明されていますが、「アサートが適切なエラーハンドリングとエラーレポートを考慮せずに済ますためのツールとして使われている」という経験上の懸念より提供されていません。適切なエラーハンドリングとはなんでしょうか。
適切なエラーハンドリングについて、「致命的ではないエラーが発生した際にクラッシュさせずに処理を継続させることである」と説明されています。
適切なエラーハンドリングを行うことは、費用対効果の高いテストを得るためにも非常に有用です。

適切なエラーハンドリングを行うことによって、致命的ではないエラーである以上一回の実行で多くのエラーをプログラマはテストの失敗結果を得ることができます。もし、適切なエラーハンドリングが行われておらず全てのエラーでクラッシュしていた場合、プログラマはエラーを直さない限り以降どのような結果が得られるかわかりません。一回の実行結果から得られる情報量がすくないため都度エラーを直しては次のエラーを調査するといった非効率的な方法でユニットテストをメンテナンスすることになります。プログラマ自身のユニットテストメンテナンスコストを鑑みても、適切なエラーハンドリングは重要です。

ユニットテストにおいて致命的なエラーとして、テスト対象を実行・検証する前準備の処理の失敗が挙げられます。もしくは、例えば、http.Handlerをテストする際に擬似リクエストに失敗した場合、以降のテストを継続することができません。このような、処理継続が不可能なものに対してはテストをクラッシュさせることが効果的です。
一方、致命的ではないエラーとしては、複数観点での検証のひとつの検証が失敗したケースなどが挙げられます。この場合、検証失敗によって以降の処理継続が不可能になるわけではありません。よって、この場合はテストをクラッシュさせないことが効果的になるわけです。

では、Goにおいて適切なエラーハンドリングを実現するためには、どのようにすればよいのでしょうか。Goでは標準の @<code>{testing}パッケージが提供する @<code>{*T.Errorf}（@<code>{*T.Error}）・@<code>{*T.Fatalf}（@<code>{*T.Fatal}）というAPIが提供しています。それらを適切に使い分けることによって、適切なエラーハンドリングを実現します。

#@# TODO 致命的なエラーとそうではないエラーが混じっているようなテスト例を用意する

@<code>{*T.Errorf}では、「対象の関数は失敗した」と記録されますが、実行は継続されます。それに対して、@<code>{*T.Fatalf}では、@<code>{*T.Errorf}と同様に「対象の関数は失敗した」ことを記録しますが、同時に実行を停止し、次のテストケースの実行へと移ります。
よって、致命的なエラーに対するハンドリングは@<code>{*T.Fatalf}で行い、そうではないエラーに対するハンドリングは、@<code>{*T.Errorf}で行います。

== 適切なエラーレポート
ユニットテストは維持するコストが発生すると「ユニットテストの考え方」にて説明しました。維持することを最小限にするためにもテストが失敗した際に「なぜ失敗したのか」について、それを見るプログラマに対してわかりやすい必要があります。そのため、適切なエラーレポートを行うことは非常に重要です。

では、適切なエラーレポートとはなんでしょうか。@<href>{https://golang.org/doc/faq#assertions}にて説明がありますが、直接的かつ適切な内容とされています。
具体的には、「施行した内容」・「どのような入力を行ったか」・「実際の結果」・「期待される結果」の４つの内容が含まれていることが望ましいです。

#@# TODO エラーメッセージの例・できていない例も含めて

== 表駆動テスト・サブテスト
Goでは非常に広く使われているテストの技法として、表駆動（table-driven）テストというものがあります。例えば、次のようなテストコードになります。

#@# TODO 表駆動テストの例

テーブル駆動テストでは、必要に応じた新たな表の項目を追加するのが簡単であり、判定ロジックの複製の不要です。そのため、このテストに対して修正・追加したいプログラマの工数を最小限である維持コストの低いテストを実現する方法になります。合わせて、テストを読む際にも、対象関数のテストパターンが表にかかれているので可読性の高いドキュメンテーションとしての価値も高いテストとなります。
判定ロジックの複製が不要な分、これまで説明してきた「適切なエラーハンドリング」・「適切なエラーレポート」に集中することができます。

== インターフェース・スタブ実装
不安定なテスト（Fragile Test）が生まれてしまうことを避ける
テストの重複を避ける
テストの速度、CI（Continuous Integration）で実行する際には注意したいポイント
3つの観点によりスタブに置き換えていくことは有効な手段。

== テストパッケージ
テストの可視性の意識
テストを実行するにあたり対象パッケージと同一パッケージにするか・ことなるパッケージにするかという2つの選択肢があります。これはそれぞれの利点があるため意思を持ってどちらかを選択する必要があります。

同一パッケージにした場合、privateな関数をテストすることができます。同一ではないパッケージにした場合は、他パッケージと同様の見え方になるので、実際の使用シーンを意識したテストケースになる利点があります。

== テストコードの再利用性を高める
毎回のテスト関数で同じコードを書いていくのは非常に面倒です。その機械的な作業を毎回行うのは、テストケースの可読性を低下させる可能性があります。また、毎回の作業になるため、適切なエラーハンドリング・エラーレポーティングの意識が疎かになる危険性があります。
テストコードの再利用性を高めるために、テストヘルパーを作成・利用する方法があります。

たとえば、次のような例を考えます。

#@# TODO ヘルパーにできるよねっていう例

xUTPでも、テストユーティリティを活用してテストの可読性を上げることをひとつの方法として説明されています。テストユーティリティには、Creation MethodとCustom Assertion Methodのふたつの種類があります。

== おわりに

本章では、費用対効果というユニットテストの考え方とGoのユニットテストの基礎を紹介しました。何か迷った際に、考え方・基礎を抑えておくことで判断にブレが少なくなります。
