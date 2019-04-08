= 費用対効果の高いユニットテストを実現するためのGoのテスト基礎

== はじめに

はじめまして、hgsgtk@<fn>{hgsgtk}です。業務ではサーバーサイドエンジニアとして、GoやPHPをメインに使ってアプリケーションの開発をしています。

//footnote[hgsgtk][@<href>{https://twitter.com/hgsgtk}]

この章では、ユニットテストについて扱います。ユニットテストを書くことは、業務でのアプリケーション開発にとって重要な要素のひとつです。ユニットテストの書き方についての記事は多数ありますが、「なぜその方法が良いのか」という考え方についての理解の場は少ないです。そのため、本章では、「どのようなユニットテストが効果的か」という点とそれをどう実現するかについて説明します。この章を読むことで、普段何気なく書くユニットテストについて立ち止まって考える機会になることを期待しています。

== ユニットテストとは
本章で扱う「ユニットテスト」という言葉について予め定義しておきます。ユニットテストとは、関数やメソッドレベルでの動作保証を行うテストです。ユニットテストを行う手段として「手動によるユニットテスト」・「自動化されたユニットテスト」の２種類があります。「手動によるユニットテスト」とは、事前に用意したユニットテスト仕様書をもとに人間の手によって行われるテストです。本章では、「自動化されたユニットテスト」を扱います。自動化されたユニットテストでは、プログラムにより実行可能なテストコードを作成します。

== Goのユニットテストの基本
まず、Goのユニットテスト作成の基本をおさえましょう。Goでは、テストを行うためのコマンドとして、@<code>{go test} というサブコマンドが用意されています。@<code>{_test.go}というサフィックスの付いたファイルを対象にしてテストを実行します。

//list[goTestExecution][go testによるテスト実行][]{
% go test github.com/hgsgtk/go-snippets/testing-codes/fizzbuzz
ok  	fizzbuzz	0.006s
//}

@<code>{_test.go} で終わるファイルは、@<code>{go build} によってビルドした場合、ビルド対象となりません。一方、 @<code>{go test} でビルドされた場合には対象パッケージの一部としてビルドされます。

テストを作成するには、 testing@<fn>{testing} パッケージをインポートしたファイルで、@<list>{sampleTestGo}のようなシグネチャを持った関数を作成します。

//footnote[testing][@<href>{https://golang.org/pkg/testing/}]

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

この場合、@<list>{sampleTestHello}のようなテストコードを作成することができます。

//list[sampleTestHello][sample_test.go][go]{
func TestSayHello(t *testing.T) {
	want := "hello"
  // SayHelloの戻り値が期待値と異なる場合エラーとして処理する
	if got := sample.SayHello(); got != want {
		t.Errorf("SayHello() = %s, want %s", got, want)
	}
}
//}

作成したユニットテストを先程紹介した @<code>{go test} で実行すると、ユニットテストが通ることが確認できます。

//list[sampleTestExecution][作成したSayHelloに対するテストが通る][]{
% go test -v github.com/hgsgtk/go-snippets/testing-codes/sample
=== RUN   TestSayHello
--- PASS: TestSayHello (0.00s)
PASS
ok  	github.com/hgsgtk/go-snippets/testing-codes/sample	0.007s
//}

このユニットテストが失敗する場合は@<list>{sampleTestExecutionFailed}のような出力結果が得られます。

//list[sampleTestExecutionFailed][作成したSayHelloに対するテストが失敗する][]{
% go test -v github.com/hgsgtk/go-snippets/testing-codes/sample
--- FAIL: TestSayHello (0.00s)
  sample_test.go:16: SayHello() = hellox, want hello
FAIL
FAIL	github.com/hgsgtk/go-snippets/testing-codes/sample	0.008s
//}

ここまでが、Goのユニットテストを作る上での基礎知識になります。基本的にはこれまで紹介した知識で最低限のテストコードを作成できるようになります。

== 費用対効果の高いユニットテストという考え方

ユニットテストを書く理由は何でしょうか。一般的には次のような理由が挙げられるでしょう。

 * バグの回避といったソフトウェア品質の向上
 * ユニットレベルのドキュメンテーション
 * 変更に対するプログラマの不安を減らす

どれもユニットテストがもたらす重要な素晴らしいメリットです。しかし、一方でユニットテストはそれ自体のコストがあります。新規にテストコードを作成するコストもあれば、既存テストを維持することも発生します。ユニットテストに不慣れな状態であればテストコード作成自体の学習コストもひとつのコストでしょう。

ここで、「費用対効果」という考え方がでてきます。ユニットテストを効果的なものにするためには、ユニットテストによる効果をユニットテスト自体のコストより高い状態を目指す必要があります。つまり、かかるコストに対して得られる利益が大きい「費用対効果の高い」テストを意識して取り組むということです。以降、費用対効果の高いユニットテストを目指していくために意識するべき点について説明します。

== 適切なエラーハンドリング
Goの特徴的な言語仕様として、 アサーションが提供されていない点が挙げられます。これは、@<href>{https://golang.org/doc/faq#assertions}にて理由が説明されていますが、「アサートが適切なエラーハンドリングとエラーレポートを考慮せずに済ますためのツールとして使われている」という経験上の懸念より提供されていません。適切なエラーハンドリングとはなんでしょうか。

適切なエラーハンドリングについて、「致命的ではないエラーが発生した際にクラッシュさせずに処理を継続させることである」と説明されています。
適切なエラーハンドリングを行うことは、費用対効果の高いテストを得るためにも非常に有用です。

では、Goにおいて適切なエラーハンドリングを実現するためには、どのようにすればよいのでしょうか。Goでは testing パッケージが提供する @<code>{*T.Error}/@<code>{*T.Errorf}@<fn>{terror}@<fn>{terrorf}・@<code>{*T.Fatal}/@<code>{*T.Fatalf}@<fn>{tfatal}@<fn>{tfatalf}というAPIが提供しています。それらを適切に使い分けることによって、適切なエラーハンドリングを実現します。

//footnote[terror][https://golang.org/pkg/testing/#T.Error]
//footnote[terrorf][https://golang.org/pkg/testing/#T.Errorf]
//footnote[tfatal][https://golang.org/pkg/testing/#T.Fatal]
//footnote[tfatalf][https://golang.org/pkg/testing/#T.Fatalf]

@<code>{*T.Errorf}では、「対象の関数は失敗した」と記録されますが、実行は継続されます。それに対して、@<code>{*T.Fatalf}では、@<code>{*T.Errorf}と同様に「対象の関数は失敗した」ことを記録しますが、同時に実行を停止し、次のテストケースの実行へと移ります。
よって、致命的なエラーに対するハンドリングは@<code>{*T.Fatalf}で行い、そうではないエラーに対するハンドリングは、@<code>{*T.Errorf}で行います。

@<list>{GetNum}を
//list[GetNum][sample.go][go]{
  package sample

  import (
  	"strconv"

  	"github.com/pkg/errors"
  )

  func GetNum(str string) (int, error) {
  	num, err := strconv.Atoi(str)
  	if err != nil {
  		return 0, errors.Wrapf(err, "GetNum failed converting %#v", str)
  	}
  	return num, nil
  }
//}

//list[TestGetNum][sample_test.go][go]{
  func TestGetNum(t *testing.T) {
  	str := "7"
  	got, err := sample.GetNum(str)
  	if err != nil {
  		t.Fatalf("GetNum(%s) caused unexpected error '%#v'", str, err)
  	}
  	want := 7
  	if got != want {
  		t.Errorf("GetNum(%s) = %d, want %d", str, got, want)
  	}
  }
//}

ユニットテストにおいて致命的なエラーとして、テスト対象を実行・検証する前準備の処理の失敗が挙げられます。もしくは、例えば、http.Handlerをテストする際に擬似リクエストに失敗した場合、以降のテストを継続することができません。このような、処理継続が不可能なものに対してはテストをクラッシュさせることが効果的です。
一方、致命的ではないエラーとしては、複数観点での検証のひとつの検証が失敗したケースなどが挙げられます。この場合、検証失敗によって以降の処理継続が不可能になるわけではありません。よって、この場合はテストをクラッシュさせないことが効果的になるわけです。

適切なエラーハンドリングを行うことによって、致命的ではないエラーである以上一回の実行で多くのエラーをプログラマはテストの失敗結果を得ることができます。適切なエラーハンドリングができていない例として@<list>{TestFizzBuzzGetMsg}のようなテストコードが挙げられます。

//list[FizzBuzzGetMsg][fizzbuzz.go][go]{
  package fizzbuzz

  import "strconv"

  func GetMsg(num int) string {
  	var res string
  	switch {
  	case num%15 == 0:
  		res = "FizzBuzz"
  	case num%5 == 0:
  		res = "Buzz"
  	case num%3 == 0:
  		res = "Fizz"
  	default:
  		res = strconv.Itoa(num)
  	}
  	return res
  }
//}

//list[TestFizzBuzzGetMsg][fizzbuzz_test.go][go]{
  func TestGetMsg(t *testing.T) {
  	var num int
  	var want string

  	num = 15
  	want = "FizzBuzz"
  	if got := fizzbuzz.GetMsg(num); want != got {
  		t.Fatalf("GetMsg(%d) = %s, want %s", num, want, got)
  	}

  	num = 5
  	want = "Buzz"
  	if got := fizzbuzz.GetMsg(num); want != got {
  		t.Fatalf("GetMsg(%d) = %s, want %s", num, want, got)
  	}

  	num = 3
  	want = "Fizz"
  	if got := fizzbuzz.GetMsg(num); want != got {
  		t.Fatalf("GetMsg(%d) = %s, want %s", num, want, got)
  	}

  	num = 1
  	want = "1"
  	if got := fizzbuzz.GetMsg(num); want != got {
  		t.Fatalf("GetMsg(%d) = %s, want %s", num, want, got)
  	}
  }
//}

すべてのエラーを、@<code>{*T.Fatalf}でハンドリングでしています。テストが通っている間はこれでも問題ないように見えるかもしれません。では、@<list>{FizzBuzzGetMsg}に２箇所バグが侵入してしまったケースを考えてみましょう。

//list[BuggFizzBuzzGetMsg][fizzbuzz.go][go]{
  package fizzbuzz

  import "strconv"

  func GetMsg(num int) string {
  	var res string
  	switch {
  	case num%16 == 0: // もともとは、num%15
  		res = "FizzBuzz"
  	case num%5 == 0:
  		res = "Buzz"
  	case num%4 == 0: // もともとは、num%3
  		res = "Fizz"
  	default:
  		res = strconv.Itoa(num)
  	}
  	return res
  }
//}

@<list>{BuggFizzBuzzGetMsg}の変更によって２箇所のバグが侵入しました。ここで、@<list>{TestFizzBuzzGetMsg}を実行してみましょう。

//list[ExecuteTestFizzBuzzGetMsg][GetMsgの戻り値検証で失敗した場合（１）][]{
=== RUN   TestGetMsg
--- FAIL: TestGetMsg (0.00s)
    fizzbuzz_test.go:47: GetMsg(15) = FizzBuzz, want Buzz
FAIL
//}

最初の１箇所のみがエラー結果として得られました。もう１つ侵入したバグに対するエラー結果を得ることはできませんでした。このテストケースではひとつの検証パターンの失敗を致命的なエラーとして扱いハンドリングしています。そのため、１回の実行によって得られる情報が少ないため、何度も修正しては現れるエラーに対して修正を繰り返さなければなりません。もし、@<code>{t.Errorf}でハンドリングしていた場合は@<list>{ExecuteTestFizzBuzzGetMsgBetter}のように１回で多くの情報を得ることができます。

//list[ExecuteTestFizzBuzzGetMsgBetter][t.Errorfでハンドリングした場合のテスト結果][]{
=== RUN   TestGetMsg
--- FAIL: TestGetMsg (0.00s)
    fizzbuzz_test.go:47: GetMsg(15) = FizzBuzz, want Buzz
    fizzbuzz_test.go:59: GetMsg(3) = Fizz, want 3
FAIL
//}

このように、適切なエラーハンドリングが行われておらず全てのエラーでクラッシュしていた場合、プログラマはエラーを直さない限り以降どのような結果が得られるかわかりません。一回の実行結果から得られる情報量がすくないため都度エラーを直しては次のエラーを調査するといった非効率的な方法でユニットテストをメンテナンスすることになります。プログラマ自身のユニットテストメンテナンスコストを鑑みても、適切なエラーハンドリングは重要です。


== 適切なエラーレポート
ユニットテストは維持するコストが発生すると「ユニットテストの考え方」にて説明しました。維持することを最小限にするためにもテストが失敗した際に「なぜ失敗したのか」について、それを見るプログラマに対してわかりやすいレポートである必要があります。そのため、適切なエラーレポートを行うことは非常に重要です。

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
