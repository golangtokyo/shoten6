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

== 適切なエラーレポート
ユニットテスト自体を維持するコストが発生する点について「費用対効果の高いユニットテストという考え方」にて説明しました。維持コストを最小限にするために意識するべき点のひとつとして、「適切なエラーレポート」という観点があります。テストが失敗した際に「なぜ失敗したのか」について、それを見るプログラマに対してわかりやすいレポートである必要があります。わかりやすいレポートにすることによって、テストの失敗に対する調査・解決にかかるコストを下げれます。

では、適切なエラーレポートとはなんでしょうか。@<href>{https://golang.org/doc/faq#assertions}にて説明がありますが、直接的かつ適切な内容とされています。
具体的には、「施行した内容」・「どのような入力を行ったか」・「実際の結果」・「期待される結果」の４つの内容が含まれていることが望ましいです。

適切なエラーレポートを知るために、逆に一度適切なエラーレポートができていないサンプルを見てましょう。@<list>{InStatusList}に対して、ユニットテストを書いてみます。

//list[InStatusList][sample.go][go]{
  func InStatusList(x string) bool {
	ls := []string{"drafted", "published"}
	for _, s := range ls {
		if x == s {
			return true
		}
	}
	return false
}
//}

//list[TestInStatusListNotReadable][適切なエラーレポートが設定できていない例][go]{
  func TestInStatusList(t *testing.T) {
  	var x string
  	var want bool

  	x = "deleted" // 実は deleted というステータスもあった
  	want = true
  	if got := sample.InStatusList(x); got != want {
  		t.Errorf("unexpected value %t", got)
  	}
  }
//}

期待した結果が得られなかった場合、どのような結果が得られたかだけをエラーレポートの内容に含めました。このテストコードを実行した場合、@<list>{ExectuteTestInStatusListNotReadable}の結果が得られます。

//list[ExectuteTestInStatusListNotReadable][適切なエラーレポートが得られない例][]{
=== RUN   TestInStatusList
--- FAIL: TestInStatusList (0.00s)
    sample_test.go:42: unexpected value false
FAIL
//}

さてこのエラーメッセージを見た時、何がわかったでしょうか。「なにやら期待していない false が返ってきたようだ」という情報は汲み取れます。しかし、「何の検証を試みて得られた結果なのか？」・「どのような入力（引数）が与えられたのか？」。「何が期待値なのか？」についてエラーレポートからはわかりません。このエラーレポートを受け取ったプログラマは対象のテストコードを慎重に読み解くことでしか何が起きているのかが理解できません。テストケースが少ないうちはこのようなエラーレポートでもメンテナンスできるかもしれませんが、ケースが多くなってきて複数箇所でエラーが発生するようになるとこのようなエラーレポートではメンテナンスが苦しくなってきます。では、より適切なエラーレポートをしているテストケースを見てみましょう。

//list[TestInStatusListReadable][適切なエラーレポートを設定した例][go]{
  func TestInStatusList(t *testing.T) {
  	var x string
  	var want bool

  	x = "deleted"
  	want = true
  	if got := sample.InStatusList(x); got != want {
  		t.Errorf("InStatusList(%s) = %t, want %t", x, got, want)
  	}
  }
//}

より適切なエラーレポートを行う一つの形式として、@<code>{f(x) = y, want z}の形式があります。f(x)では試みた結果と入力、yは得られた結果、zは期待値を表します。@<list>{TestInStatusListReadable}の例では、@<code>{t.Errorf("InStatusList(%s) = %t, want %t", x, got, want)}にて、エラーレポートしています。これを実行すると@<list>{ExecuteTestInStatusListReadable}の結果が得られます。

//list[ExecuteTestInStatusListReadable][より適切なエラーレポートが得られた例][]{
  === RUN   TestInStatusList
  --- FAIL: TestInStatusList (0.00s)
      sample_test.go:42: InStatusList(deleted) = false, want true
  FAIL
//}

@<list>{ExectuteTestInStatusListNotReadable}の結果と比較して、エラーレポートから得られる情報量が増え、「@<code>{InStatusList}の検証を試み deleted という文字列を入力した際に false という結果が得られたが、期待値は true であった。」と直接的かつ十分な情報量を読み取ることができます。

== 適切なエラーハンドリング
Goの特徴的な言語仕様として、 アサーションが提供されていない点が挙げられます。これは、@<href>{https://golang.org/doc/faq#assertions}にて理由が説明されていますが、「アサートが適切なエラーハンドリングとエラーレポートを考慮せずに済ますためのツールとして使われている」という経験上の懸念より提供されていません。適切なエラーハンドリングとはなんでしょうか。

適切なエラーハンドリングについて、「致命的ではないエラーが発生した際にクラッシュさせずに処理を継続させることである」と説明されています。
適切なエラーハンドリングを行うことは、費用対効果の高いテストを得るためにも非常に有用です。

では、Goにおいて適切なエラーハンドリングを実現するためには、どのようにすればよいのでしょうか。Goでは testing パッケージが提供する @<code>{*T.Error}@<fn>{terror}/@<code>{*T.Errorf}@<fn>{terrorf}・@<code>{*T.Fatal}@<fn>{tfatal}/@<code>{*T.Fatalf}@<fn>{tfatalf}というAPIが提供しています。それらを適切に使い分けることによって、適切なエラーハンドリングを実現します。

//footnote[terror][https://golang.org/pkg/testing/#T.Error]
//footnote[terrorf][https://golang.org/pkg/testing/#T.Errorf]
//footnote[tfatal][https://golang.org/pkg/testing/#T.Fatal]
//footnote[tfatalf][https://golang.org/pkg/testing/#T.Fatalf]

@<code>{*T.Errorf}では、「対象の関数は失敗した」と記録されますが、実行は継続されます。それに対して、@<code>{*T.Fatalf}では、@<code>{*T.Errorf}と同様に「対象の関数は失敗した」ことを記録しますが、同時に実行を停止し、次のテストケースの実行へと移ります。
よって、致命的なエラーに対するハンドリングは@<code>{*T.Fatalf}で行い、そうではないエラーに対するハンドリングは、@<code>{*T.Errorf}で行います。

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
  	if got := fizzbuzz.GetMsg(num); got != want {
  		t.Fatalf("GetMsg(%d) = %s, want %s", num, got, want)
  	}

  	num = 5
  	want = "Buzz"
  	if got := fizzbuzz.GetMsg(num); got != want {
  		t.Fatalf("GetMsg(%d) = %s, want %s", num, got, want)
  	}

  	num = 3
  	want = "Fizz"
  	if got := fizzbuzz.GetMsg(num); got != want {
  		t.Fatalf("GetMsg(%d) = %s, want %s", num, got, want)
  	}

  	num = 1
  	want = "1"
  	if got := fizzbuzz.GetMsg(num); got != want {
  		t.Fatalf("GetMsg(%d) = %s, want %s", num, got, want)
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
    fizzbuzz_test.go:47: GetMsg(15) = Buzz, want FizzBuzz
FAIL
//}

最初の１箇所のみがエラー結果として得られました。もう１つ侵入したバグに対するエラー結果を得ることはできませんでした。このテストケースではひとつの検証パターンの失敗を致命的なエラーとして扱いハンドリングしています。そのため、１回の実行によって得られる情報が少ないため、何度も修正しては現れるエラーに対して修正を繰り返さなければなりません。もし、@<code>{t.Errorf}でハンドリングしていた場合は@<list>{ExecuteTestFizzBuzzGetMsgBetter}のように１回で多くの情報を得ることができます。

//list[ExecuteTestFizzBuzzGetMsgBetter][t.Errorfでハンドリングした場合のテスト結果][]{
=== RUN   TestGetMsg
--- FAIL: TestGetMsg (0.00s)
    fizzbuzz_test.go:47: GetMsg(15) = Buzz, want FizzBuzz
    fizzbuzz_test.go:59: GetMsg(3) = 3, want Fizz
FAIL
//}

このように、適切なエラーハンドリングが行われておらず全てのエラーでクラッシュしていた場合、プログラマはエラーを直さない限り以降どのような結果が得られるかわかりません。一回の実行結果から得られる情報量がすくないため都度エラーを直しては次のエラーを調査するといった非効率的な方法でユニットテストをメンテナンスすることになります。プログラマ自身のユニットテストメンテナンスコストを鑑みても、適切なエラーハンドリングは重要です。

== テーブル駆動テスト・サブテスト
Goでは非常に広く使われているテストの技法として、テーブル駆動テストというものがあります。@<list>{FizzBuzzGetMsg}をテーブル駆動テストで書いてみましょう。

//list[TestFizzBuzzGetMsgTableDriven][テーブル駆動テスト][go]{
  func TestGetMsg(t *testing.T) {
  	tests := []struct {
  		num  int
  		want string
  	}{
  		{
  			num:  15,
  			want: "FizzBuzz",
  		},
  		{
  			num:  5,
  			want: "Buzz",
  		},
  		{
  			num:  3,
  			want: "Fizz",
  		},
  		{
  			num:  1,
  			want: "1",
  		},
  	}
  	for _, tt := range tests {
  		if got := fizzbuzz.GetMsg(tt.num); got != tt.want {
  			t.Errorf("GetMsg(%d) = %s, want %s", tt.num, got, tt.want)
  		}
  	}
  }
//}

テーブル駆動テストでは、必要に応じた新たな表の項目を追加するのが簡単であり、判定ロジックの複製の不要です。そのため、このテストに対して修正・追加したいプログラマの工数を最小限である維持コストの低いテストを実現する方法になります。合わせて、テストを読む際にも、対象関数のテストパターンが表にかかれているので可読性の高いドキュメンテーションとしての価値も高いテストとなります。さらに、判定ロジックの複製が不要な分、これまで説明してきた「適切なエラーハンドリング」・「適切なエラーレポート」に集中することができます。

加えて、サブテストを使うことができます。サブテストは@<code>{*T.Run}@<fn>{trun}を使用することによって実現します。@<list>{TestFizzBuzzGetMsgTableDriven}をサブテストを使うように書き換えます。

//footnote[trun][https://golang.org/pkg/testing/#T.Run]

//list[TestFizzBuzzGetMsgSubTest][サブテスト][go]{
func TestGetMsg(t *testing.T) {
	tests := []struct {
		desc string
		num  int
		want string
	}{
		{
			desc: "divisible by 15",
			num:  15,
			want: "FizzBuzz",
		},
		{
			desc: "divisible by 5",
			num:  5,
			want: "Buzz",
		},
		{
			desc: "divisible by 3",
			num:  3,
			want: "Fizz",
		},
		{
			desc: "not divisible",
			num:  1,
			want: "1",
		},
	}
	for _, tt := range tests {
		t.Run(tt.desc, func(t *testing.T) {
			if got := fizzbuzz.GetMsg(tt.num); got != tt.want {
				t.Errorf("GetMsg(%d) = %s, want %s", tt.num, got, tt.want)
			}
		})
	}
}
//}

@<list>{TestFizzBuzzGetMsgSubTest}を実行します。

//list[ExectuteTestFizzBuzzGetMsgSubTest][サブテストを実行する][go]{
=== RUN   TestGetMsg
--- PASS: TestGetMsg (0.00s)
=== RUN   TestGetMsg/divisible_by_15
    --- PASS: TestGetMsg/divisible_by_15 (0.00s)
=== RUN   TestGetMsg/divisible_by_5
    --- PASS: TestGetMsg/divisible_by_5 (0.00s)
=== RUN   TestGetMsg/divisible_by_3
    --- PASS: TestGetMsg/divisible_by_3 (0.00s)
=== RUN   TestGetMsg/not_divisible
    --- PASS: TestGetMsg/not_divisible (0.00s)
PASS
//}

サブテストを活用することは費用対効果の高いテストの実現に有用です。まず、それぞれのテストケースにテストケース名を与えることができます。これは、そのテストコードを読むプログラマが意図を理解しやすい可読性の高いテストコードに繋がります。また、特定のサブテストのみを実行することができます。たとえば、@<code>{divisible_by_15}のみを実行したい場合は、@<code>{--run}オプションをtestコマンドにわたすことで実現できます。特定のサブテストのみを実行できることによりデバッグ時に必要最小限のテスト実行で済むためデバッグコストの削減に繋がります。

//list[ExectuteSpeficiedTestFizzBuzzGetMsgSubTest][特定のサブテストを実行する][]{
  % go test github.com/hgsgtk/go-snippets/testing-codes/fizzbuzz --run TestGetMsg/divisible_by_15 -v
  === RUN   TestGetMsg
  === RUN   TestGetMsg/divisible_by_15
  --- PASS: TestGetMsg (0.00s)
      --- PASS: TestGetMsg/divisible_by_15 (0.00s)
  PASS
  ok  	github.com/hgsgtk/go-snippets/testing-codes/fizzbuzz	0.008s
//}

さらに、@<code>{*T.Parallel}@<fn>{tparallel}を合わせて使うことで並行でのテスト実行が可能になります。並行処理が可能になることでテストケース全体の実行時間を短縮することができます。

//footnote[tparallel][https://golang.org/pkg/testing/#T.Parallel]

== テストパッケージ
テストコードのパッケージ名について、対象のパッケージと同一のパッケージ名にするか、@<code>{xxx_test}という外部テストパッケージを行う２つの選択肢があります。
外部テストパッケージを選択した場合、対象とは異なるパッケージになるため、パッケージの非公開機能などにはアクセスできなくなります。しかし、@<code>{export_test.go}を介してアクセスする方法もあります。

#@# export_testの例を書く

費用対効果の高いテストを目指すために、テストパッケージはどちらの方法を選択するべきなのでしょうか。これについて知るために両者のメリット・デメリットを知る必要があります。これを考えるための材料として、ブラックボックステスト・ホワイトボックステストという２つの概念をおさえましょう。

テストを分類する一つの方法として、テスト対象の内部の動きをどれだけ知っているかという観点があります。この観点で、ブラックボックステスト・ホワイトボックスという２つのテストに分類できます。ブラックボックステストは、パッケージがAPIとして公開している以外はそのパッケージについて関して何も知らないことを前提とします。対象的にホワイトボックステストはパッケージの内部機能にアクセスできます。

ブラックボックステストは、「壊れやすいテスト（Fragile Test）」@<fn>{fragile-test}になりにくい利点があります。壊れやすいテストとは、動作コードの変更に敏感に反応してテストが落ちてしまうようなテストのことをいいます。ここでいう「敏感に」とは、外部への振る舞いを変えない内部の処理詳細の変更に対してもテストが落ちてしまうということです。変更検出（change detecter）テストあるいは現状維持（status quo）テストと呼ばれることもあります。壊れやすいテストは、必要以上の変更頻度を発生させ、テストの維持コストの増大に繋がります。ブラックボックステストを行うことは、そのような維持コストの高いテストの発生を防ぐ利点があります。

//footnote[fragile-test][http://xunitpatterns.com/Fragile%20Test.html]

テストの可視性の意識

同一パッケージにした場合、privateな関数をテストすることができます。同一ではないパッケージにした場合は、他パッケージと同様の見え方になるので、実際の使用シーンを意識したテストケースになる利点があります。

== テストコードの再利用性を高める
#@# まだだめだぁ

毎回のテスト関数で同じコードを書いていくのは非常に面倒です。その機械的な作業を毎回行うのは、テストケースの可読性を低下させる可能性があります。また、毎回の作業になるため、適切なエラーハンドリング・エラーレポーティングの意識が疎かになる危険性があります。
テストコードの再利用性を高めるために、テストヘルパーを作成・利用する方法があります。

たとえば、次のような例を考えます。

#@# TODO ヘルパーにできるよねっていう例

xUTPでも、テストユーティリティを活用してテストの可読性を上げることをひとつの方法として説明されています。テストユーティリティには、Creation MethodとCustom Assertion Methodのふたつの種類があります。

== おわりに

本章では、費用対効果というユニットテストの考え方とGoのユニットテストの基礎を紹介しました。何か迷った際に、考え方・基礎を抑えておくことで判断にブレが少なくなります。
