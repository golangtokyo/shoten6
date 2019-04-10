= 費用対効果の高いユニットテストを実現するためのGoの基礎技法

== はじめに

はじめまして、hgsgtk@<fn>{hgsgtk}です。BASE BANK株式会社@<fn>{basebank} Dev Divisionにてサーバーサイドエンジニアをしています。業務では、GoやPHPをメインに使ってアプリケーションの開発をしています。

//footnote[basebank][@<href>{https://basebank.jp/}]
//footnote[hgsgtk][@<href>{https://twitter.com/hgsgtk}]

この章では、ユニットテストについて扱います。ユニットテストを書くことは、業務でのアプリケーション開発にとって重要な要素のひとつです。ユニットテストの書き方についての記事は多数ありますが、"なぜその方法が良いのか"についての理解の場は少ないです。そのため、本章では、"どのようなユニットテストが効果的か"と"それをどう実現するか"について説明します。この章を読むことで、普段何気なく書くユニットテストについて立ち止まって考える機会になることを期待しています。

== ユニットテストとは

本章で扱う"ユニットテスト"について予め定義しておきます。ユニットテストとは、関数やメソッドレベルでの動作保証を行うテストです。ユニットテストを行う手段として手動によるユニットテスト・自動化されたユニットテストの２種類があります。手動によるユニットテストとは、事前に用意したユニットテスト仕様書をもとに人間の手によって行われるユニットテストです。対照的に、自動化されたユニットテストでは、プログラムにより実行可能なテストコードを作成し実行します。本章で扱うのは自動化されたユニットテストですので、"テストコードをどのように書いていくか"について焦点をあてていきます。

== Goのユニットテストの基本

まず、Goのユニットテスト作成の基本をおさえましょう。Goでは、ユニットテストを行うためのコマンドとして、@<code>{go test}@<fn>{gotestcmd} というサブコマンドが用意されています。このコマンドは、@<code>{_test.go}のサフィックスが付いたファイルを対象にしてユニットテストを実行します。

//footnote[gotestcmd][@<href>{https://golang.org/pkg/cmd/go/internal/test/}]

//list[goTestExecution][go testによるユニットテスト実行][]{
% go test fizzbuzz
ok  	fizzbuzz	0.006s
//}

@<code>{_test.go}で終わるファイルは、@<code>{go build}@<fn>{gobuild}によってビルドした場合、ビルド対象となりません。一方、 @<code>{go test}によってビルドされた場合には、対象パッケージの一部としてビルドされます。

//footnote[gobuild][@<href>{https://golang.org/cmd/go/#hdr-Compile_packages_and_dependencies}]

テストコードを作成するには、 testing@<fn>{testing} パッケージをインポートしたファイルで、@<list>{sampleTestGo}のようなシグネチャを持った関数を作成します。

//footnote[testing][@<href>{https://golang.org/pkg/testing/}]

//list[sampleTestGo][sampleパッケージのName()に対するユニットテスト][go]{
package sample_test

import "testing"

func TestName(t *testing.T) {
	// ...
}
//}

テスト関数名は、 @<code>{Test} で始め、以降の接頭辞 @<code>{Name} は大文字で始める必要があります。また、引数定義には、@<code>{*testing.T}を設定します。

ひとつ例を見てましょう。hello と返却する@<code>{SayHello()}関数があるとします。

//list[sampleHello][SayHello()の実装][go]{
package sample

func SayHello() string {
	return "hello"
}
//}

この場合、@<list>{sampleTestHello}のようなテストコードを作成することができます。

//list[sampleTestHello][SayHello()に対するテストコード][go]{
func TestSayHello(t *testing.T) {
	want := "hello"
	// SayHelloの戻り値が期待値と異なる場合エラーとして処理する
	if got := sample.SayHello(); got != want {
		t.Errorf("SayHello() = %s, want %s", got, want)
	}
}
//}

作成したユニットテストを先程紹介した @<code>{go test} で実行すると、ユニットテストが通ることが確認できます。（詳細結果を表示するため、@<code>{-v}オプションを設定して実行します。）

//list[sampleTestExecution][TestSayHello()が成功する][]{
% go test -v sample
=== RUN   TestSayHello
--- PASS: TestSayHello (0.00s)
PASS
ok  	sample	0.007s
//}

このユニットテストが失敗する場合は@<list>{sampleTestExecutionFailed}のような出力結果が得られます。

//list[sampleTestExecutionFailed][TestSayHello()が失敗する][]{
% go test -v sample
--- FAIL: TestSayHello (0.00s)
  sample_test.go:16: SayHello() = hellox, want hello
FAIL
FAIL	sample	0.008s
//}

ここまでが、Goのユニットテストを作る上での基本知識になります。基本的にはこれまで紹介した知識で最低限のテストコードを作成できるようになります。

== 費用対効果の高いユニットテストとは

ユニットテストを書く理由は何でしょうか。一般的には次のような理由が挙げられるでしょう。

 * バグの回避といったソフトウェア品質の向上
 * ユニットレベルのドキュメンテーション
 * 変更に対するプログラマの不安を減らす

どれもユニットテストがもたらす重要な素晴らしいメリットです。しかし、一方でユニットテストはそれ自体のコストがあります。具体的には次のようなコストが挙げられます。

* 新規テストコードの作成コスト
* 既存テストコードの維持コスト
* テストコード作成自体の学習コスト

当然のことながらコードを書いているので、ユニットテストを作成・維持するためのコストは発生します。

ここで、"費用対効果"の考え方がでてきます。ユニットテストがソフトウェア開発において効果的であるためには、ユニットテストによる効果がユニットテスト自体のコストより高い状態を保つ必要があります。つまり、かかるコストに対して得られる利益が大きい"費用対効果の高い"ユニットテストを意識するのです。

このユニットテストの効果とコストの関係性について、書籍『xUnit Test Patterns: Refactoring Test Code』@<fn>{xutpLink}では、"Economics of Test Automation"という言葉で説明されています。

//footnote[xutpLink][@<href>{https://www.amazon.co.jp/dp/0131495054}]

ソフトウェア開発プロジェクトにおいて、初めてユニットテストを書き始める初期時点では、新しい技術に対する学習・実践コストが発生します。それにより、ユニットテストのコストが嵩む時期が訪れます。しかし、じきにユニットテストに対する習熟や効率化により追加コストが落ち着いてくると、@<img>{hgsgtk-test-economical-graph}のように、ユニットテストがもたらす効果である節約コストと相殺されていきます。

//image[hgsgtk-test-economical-graph][費用対効果の高いユニットテストの場合][scale=0.5]{
//}

しかし、費用対効果の低いユニットテストが多い場合、ユニットテストがもたらす節約コストが少ないわりに、ユニットテスト自体のコストが大きくなり、@<img>{hgsgtk-test-uneconomical-graph}のようにソフトウェア開発のトータルのコストが増大していきます。

//image[hgsgtk-test-uneconomical-graph][費用対効果の低いユニットテストの場合][scale=0.5]{
//}

ソフトウェア開発のコスト全体を考慮すると、費用対効果の高いユニットテストになるように意識することは重要です。以降、費用対効果の高いユニットテストを目指すために意識するべき点について説明します。

== 適切なエラーレポート

ユニットテスト自体のコストの発生について"費用対効果の高いユニットテストとは"にて説明しました。その中でも、ユニットテストの維持コストを最小限におさえるために意識するべき点のひとつとして、適切なエラーレポートがあります。
ユニットテストの失敗時にエラーレポートが"なぜ失敗したのか"をプログラマに対してわかりやすく伝えている場合、そのユニットテストは適切なエラーレポートができていると言えます。わかりやすいエラーレポートにすることによって、ユニットテストの失敗に対する調査・解決にかかるコストが下げり、ユニットテストを維持するコストの削減に繋がります。

では、適切なエラーレポートとはなんでしょうか。Goの公式FAQの"Why does Go not have assertions?"@<fn>{goFaqAssertions}では、"直接的かつ適切な内容"と説明されています。
具体的には、"施行した内容"・"どのような入力を行ったか"・"実際の結果"・"期待される結果"の４つの内容が含まれていることが望ましいとされています。

適切なエラーレポートを知るために、一度適切なエラーレポートができていない例を見てましょう。

//list[InStatusList][特定の文字列リストに引数が一致するか判定するInStatusList()][go]{
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

@<list>{InStatusList}に対して、適切なエラーレポートができていないユニットテストを書いてみます。

//list[TestInStatusListNotReadable][適切なエラーレポートが設定できていない例][go]{
func TestInStatusList(t *testing.T) {
	var x string
	var want bool

	x = "deleted" // 実は deleted ステータスもあった
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

さて、このエラーメッセージを見た時、何がわかったでしょうか。"なにやら期待していない false が返ってきたようだ"という情報は汲み取れます。しかし、"何の検証を試みて得られた結果なのか"・"どのような入力（引数）が与えられたのか"・"何が期待値なのか"についてエラーレポートからは分かりません。このエラーレポートを受け取ったプログラマは、失敗したテストコード・テスト対象の動作コードを慎重に読み解くことでしか何が起きているのか理解できません。テストケースが少ないうちはこのようなエラーレポートでもメンテナンスできるかもしれませんが、ケースが多くなってきて複数箇所でエラーが発生するようになるとこのようなエラーレポートではメンテナンスが苦しくなってきます。
では、より適切なエラーレポートをしているテストケースを見てみましょう。

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

より適切なエラーレポートを行う一つの形式として、"f(x) = y, want z"の形式があります。f(x)では試みた結果と入力、yは得られた結果、zは期待値を表します。@<list>{TestInStatusListReadable}の例では、@<code>{t.Errorf("InStatusList(%s) = %t, want %t", x, got, want)}にて、エラーレポートしています。これを実行すると@<list>{ExecuteTestInStatusListReadable}の結果が得られます。

//list[ExecuteTestInStatusListReadable][より適切なエラーレポートが得られた例][]{
=== RUN   TestInStatusList
--- FAIL: TestInStatusList (0.00s)
    sample_test.go:42: InStatusList(deleted) = false, want true
FAIL
//}

@<list>{ExectuteTestInStatusListNotReadable}の結果と比較して、エラーレポートから得られる情報量が増え、"@<code>{InStatusList()}の検証を試み deleted 文字列を入力した際に false が得られたが、期待値は true であった。"と直接的かつ十分な情報量を読み取ることができます。理想的には、保守するプログラマがユニットテストの失敗を解明するためにソースコードを読む必要がないエラーレポートにするべきです。

== 適切なエラーハンドリング

Goの特徴的な言語仕様として、 アサーションが提供されていない点が挙げられます。これは、Goの公式FAQの"Why does Go not have assertions?"@<fn>{goFaqAssertions}にて理由が説明されていますが、"アサートが適切なエラーハンドリングとエラーレポートを考慮せずに済ますためのツールとして使われている"という懸念より提供されていません。このことからも、Goのユニットテストにおいて、適切なエラーハンドリングが重要なことがわかります。では、適切なエラーハンドリングとはなんでしょうか。適切なエラーハンドリングとは、致命的ではないエラーが発生した際にクラッシュさせずに処理を継続させることを示します。

//footnote[goFaqAssertions][@<href>{https://golang.org/doc/faq#assertions}]

Goにおいて適切なエラーハンドリングを実現するための具体的な方法について説明していきます。Goではエラーハンドリングのために testing パッケージが@<code>{*testing.T.Error}@<fn>{terror}/@<code>{*testing.T.Errorf}@<fn>{terrorf}・@<code>{*testing.T.Fatal}@<fn>{tfatal}/@<code>{*testing.T.Fatalf}@<fn>{tfatalf}を提供しています。それらを適切に使い分けることによって、適切なエラーハンドリングを実現します。

//footnote[terror][@<href>{https://golang.org/pkg/testing/#T.Error}]
//footnote[terrorf][@<href>{https://golang.org/pkg/testing/#T.Errorf}]
//footnote[tfatal][@<href>{https://golang.org/pkg/testing/#T.Fatal}]
//footnote[tfatalf][@<href>{https://golang.org/pkg/testing/#T.Fatalf}]

@<code>{*testing.T.Error}/@<code>{*testing.T.Errorf}では、"対象のテストケースが失敗した"と記録されますが、実行は継続されます。それに対して、@<code>{*testing.T.Fatal}/@<code>{*testing.T.Fatalf}では、"対象のテストケースが失敗した"ことを記録した上で、同時に実行を停止し、後続のユニットテストの実行へと移ります。
よって、処理の継続において致命的なエラーに対するハンドリングは@<code>{*testing.T.Fatal}/@<code>{*testing.T.Fatalf}で行い、そうではないエラーに対するハンドリングは、@<code>{*testing.T.Error}/@<code>{*testing.T.Errorf}で行います。
ひとつ、@<code>{*testing.T.Fatalf}と@<code>{*testing.T.Errorf}を使い分ける例を見てましょう。

//list[OkHandler][OKを返すHTTPハンドラーOkHandler()][go]{
package sample

import (
	"encoding/json"
	"net/http"
)

func OkHandler(w http.ResponseWriter, r *http.Request) {
	type Body struct {
		Status string `json:"status"`
	}
	body := Body{Status: "OK"}
	w.WriteHeader(http.StatusOK)
	if err := json.NewEncoder(w).Encode(body); err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		return
	}
}
//}

@<list>{OkHandler}に対して@<code>{*testing.T.Fatalf}と@<code>{*testing.T.Errorf}両方を使用してテストコードを書きます。

//list[TestOkHandler][OkHandler()に対するテストコード][go]{
package sample_test

import (
	"io/ioutil"
	"net/http/httptest"
	"testing"

	"github.com/hgsgtk/go-snippets/testing-codes/sample"
)

func TestOkHandler(t *testing.T) {
	w := httptest.NewRecorder()
	r := httptest.NewRequest("GET", "/sample", nil)

	sample.OkHandler(w, r)
	res := w.Result()
	defer res.Body.Close()

	b, err := ioutil.ReadAll(res.Body)
	if err != nil {
		t.Fatalf("ioutil.ReadAll() caused unexpected error '%#v'", err)
	}
	const expected = "{\"status\":\"OK\"}\n"
	if got := string(b); got != expected {
		t.Errorf("OkHandler response = '%#v', want '%#v'", got, expected)
	}
}
//}

@<list>{TestOkHandler}では、対象のHTTPハンドラである@<code>{OkHandler()}のレスポンスボディを@<code>{ioutil.ReadAll()}で読み取った際にエラーが発生するケースがありますが、ここでエラーが発生した場合は後続のレスポンスボディの値検証は不可なため、@<code>{*testing.T.Fatalf}を使ってテストをクラッシュさせています。

ユニットテストにおいて致命的なエラーとして、テスト対象を実行・検証する前準備の処理の失敗が挙げられます。このような、処理の継続が不可能なものに対してはユニットテストをクラッシュさせることが効果的です。
一方、致命的ではないエラーとしては、複数観点での検証のひとつの検証が失敗したケースなどが挙げられます。この場合、検証失敗によって以降の処理継続が不可能になるわけではありません。よって、この場合はユニットテストをクラッシュさせないことが効果的になるわけです。

適切なエラーハンドリングを行うことによって、一回のユニットテスト実行で多くのエラー結果を得ることができます。この利点を実感するために、適切なエラーハンドリングができていない例を見てましょう。

//list[FizzBuzzGetMsg][数値に対してFizzBuzz判定をするfizzbuzz.GetMsg][go]{
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

@<list>{FizzBuzzGetMsg}に対して適切なエラーハンドリングをしないテストコードを書いてみます。

//list[TestFizzBuzzGetMsg][適切なエラーハンドリングができていない例][go]{
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

すべてのエラーを、@<code>{*testing.T.Fatalf}でハンドリングでしています。ユニットテストが通っている間はこれでも問題ないように見えるかもしれません。では、@<list>{FizzBuzzGetMsg}に２箇所バグが侵入してしまったケースを考えてみましょう。

//list[BugFizzBuzzGetMsg][２箇所バグが侵入したfizzbuzz.GetMsg][go]{
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

@<list>{BugFizzBuzzGetMsg}の変更によって２箇所のバグが侵入しました。ここで、@<list>{TestFizzBuzzGetMsg}を実行してみましょう。

//list[ExecuteTestFizzBuzzGetMsg][２箇所のバグのうち１箇所のみ失敗結果として表示される][]{
=== RUN   TestGetMsg
--- FAIL: TestGetMsg (0.00s)
    fizzbuzz_test.go:47: GetMsg(15) = Buzz, want FizzBuzz
FAIL
//}

最初の１箇所のみがエラー結果として得られました。もう１つ侵入したバグに対するエラー結果を得ることはできませんでした。このテストコードでは、処理の継続にとって致命的なエラーではないものもすべて@<code>{*testing.T.Fatalf}でハンドリングしています。結果、１回の実行によって得られる情報が少ないため、"何度も修正しては現れる別のエラーに対して修正を繰り返さなければならない"状況を作ってしまう可能性があります。もし、@<code>{*testing.T.Errorf}でハンドリングしていた場合は@<list>{ExecuteTestFizzBuzzGetMsgBetter}のように１回で多くの情報を得ることができます。

//list[ExecuteTestFizzBuzzGetMsgBetter][適切なエラーハンドリングした場合のテスト結果][]{
=== RUN   TestGetMsg
--- FAIL: TestGetMsg (0.00s)
    fizzbuzz_test.go:47: GetMsg(15) = Buzz, want FizzBuzz
    fizzbuzz_test.go:59: GetMsg(3) = 3, want Fizz
FAIL
//}

このように、適切なエラーハンドリングが行われておらず全てのエラーでクラッシュしていた場合、プログラマはエラーを直さない限り、クラッシュした箇所以降を実行した際にどのような結果が得られるかわかりません。一回の実行結果から得られる情報量がすくないため、保守するプログラマは"都度エラーを直しては次のエラーを調査する"といった方法を選択することになります。ユニットテスト維持コストを鑑みても、適切なエラーハンドリングは意識する必要があります。

== テーブル駆動テスト・サブテスト

=== テーブル駆動テスト

Goでは非常に広く使われているユニットテストの技法として、テーブル駆動テスト@<fn>{wikiTableDrivenDevelopment}があります。@<list>{FizzBuzzGetMsg}をテーブル駆動テストで書いてみましょう。

//footnote[wikiTableDrivenDevelopment][@<href>{https://github.com/golang/go/wiki/TableDrivenTests}]

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

テーブル駆動テストでは、必要に応じた新たな表の項目を追加するのが簡単であり、判定ロジックの複製の不要です。そのため、テストコードに対する修正・追加コストの低いユニットテストを実現する方法になります。あわせて、テストコードを読む際にも、対象関数のテストパターンが表にかかれているので可読性の高いユニットテストとなります。さらに、判定ロジックの複製が不要な分、これまで説明してきた適切なエラーレポート・エラーハンドリングに集中することができます。

=== サブテスト

テーブル駆動テストとあわせてよく使用されるものとして、サブテスト@<fn>{subtest}があります。サブテストは@<code>{*testing.T.Run}@<fn>{trun}を使用することによって作ることができます。先程の@<list>{TestFizzBuzzGetMsgTableDriven}をサブテストを作るように書き換えてみましょう。

//footnote[subtest][@<href>{https://blog.golang.org/subtests}]
//footnote[trun][@<href>{https://golang.org/pkg/testing/#T.Run}]

//list[TestFizzBuzzGetMsgSubTest][サブテストを作る例][go]{
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

@<code>{*testing.T.Run}の第一引数で渡した文字列がサブテスト名になります。そのため@<list>{TestFizzBuzzGetMsgSubTest}では、テーブルに項目 desc を追加してそれぞれのテストケースに名前をつけるようにしました。このテストを実行してみましょう。

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

テスト実行結果にサブテスト名が表示されるようになりました。サブテストにすることで特定のサブテストのみを実行することができます。たとえば、@<code>{divisible_by_15}のみを実行したい場合は、@<code>{--run}オプションをtestコマンドにわたすことで実現できます。

//list[ExectuteSpeficiedTestFizzBuzzGetMsgSubTest][特定のサブテストを実行する][]{
% go test fizzbuzz -run TestGetMsg/divisible_by_15 -v
=== RUN   TestGetMsg
=== RUN   TestGetMsg/divisible_by_15
--- PASS: TestGetMsg (0.00s)
    --- PASS: TestGetMsg/divisible_by_15 (0.00s)
PASS
ok  	fizzbuzz	0.008s
//}

サブテストを活用することで、それぞれのテストケースにテストケース名を与えることができることを紹介しました。これは、保守するために読むプログラマに対して意図を伝えやすいテストコードの作成に繋がります。また、特定のサブテストのみを実行できることによりデバッグ時に必要最小限のユニットテスト実行で済むためデバッグコストの削減に繋がります。

さらに、@<code>{*testing.T.Parallel}@<fn>{tparallel}を合わせて使うことで並行でのユニットテスト実行が可能になります。並行処理が可能になることでテストケース全体の実行時間を短縮することができます。

//footnote[tparallel][@<href>{https://golang.org/pkg/testing/#T.Parallel}]

== テストヘルパー

費用対効果の高いユニットテストを目指す上で、テストコードの可読性やテストコードの追加のしやすさは重要な要素です。テストコードの可読性はドキュメンテーションとしての価値を高め、追加のしやすさは新規テストコードの作成コストの削減に繋がります。これら２つを狙う上でテストヘルパーを作成・利用する方法があります。実際に使用する例を見ていきましょう。

//list[GetTomorrow][GetTomorrow][go]{
func GetTomorrow(tm time.Time) time.Time {
	return tm.AddDate(0, 0, 1)
}
//}

@<list>{GetTomorrow}の関数のユニットテストを作成します。まずは、テストヘルパーを使用しない場合のテストケースを示します。

//list[TestGetTomorrowBefore][テストヘルパーを使用しない場合][go]{
func TestGetTomorrow(t *testing.T) {
	jst, err := time.LoadLocation("Asia/Tokyo")
	if err != nil {
		t.Fatalf("Failed to load JST time Location")
	}
	tm := time.Date(2019, time.April, 14, 0, 0, 0, 0, jst)

	want := tm.AddDate(0, 0, 1)
	got := sample.GetTomorrow(tm)
	if diff := cmp.Diff(got, want); diff != "" {
		t.Errorf("GetTomorrow() differs: (-got +want)\n%s", diff)
	}
}
//}

@<list>{TestGetTomorrowBefore}では、テスト対象である@<code>{GetTomorrow}実行前に Asia/Tokyo のtimezoneを用意して失敗した場合はエラーハンドリングしています。これをテストヘルパーに切り出してみます。

===[column] 値比較のライブラリgithub.com/google/go-cmp
@<list>{TestGetTomorrowBefore}で@<code>{cmp.Diff}関数にて値を比較しています。これは、github.com/google/go-cmp@<fn>{gocmp}というGoogle非公式の値比較ライブラリが提供する機能を利用しています。github.com/google/go-cmpは、JSON形式の文字列や構造体の値をテストする際に便利なライブラリなので、テストコードでの値比較手段として採用を検討してみるとよいでしょう。

//footnote[gocmp][@<href>{https://github.com/google/go-cmp}]
===[/column]

//list[TestHelperGetJstLocation][テストヘルパーを作成する][go]{
package testutil

import (
	"testing"
	"time"
)

func GetJstLocation(t *testing.T) *time.Location {
	t.Helper()

	jst, err := time.LoadLocation("Asia/Tokyo")
	if err != nil {
		t.Fatalf("Failed to load JST time Location")
	}
	return jst
}
//}

testing パッケージには、@<code>{*testing.T.Helper}@<fn>{thelper}があります。@<code>{*testing.T.Helper}を呼び出すことでテストヘルパー関数として扱われます。そのため、@<list>{TestHelperGetJstLocation}の@<code>{GetJstLocation}では、引数に@<code>{*testing.T}を受け取り@<code>{*testing.T.Helper}を呼び出しています。また、エラーが発生した場合はエラーを戻り値として返すのではなく、@<code>{*testing.T.Fatalf}でテストを失敗させています。これは、このテストヘルパーを呼び出すテストコードからエラーハンドリングを排除できるメリットがあります。作成したテストヘルパーを利用して@<list>{TestGetTomorrowBefore}を書き換えてみます。

//footnote[thelper][@<href>{https://golang.org/pkg/testing/#T.Helper}]

//list[TestGetTomorrowAfter][テストヘルパーを使用する例][go]{
func TestGetTomorrowUsingCmp(t *testing.T) {
	tm := time.Date(2019, time.April, 14, 0, 0, 0, 0, testutil.GetJstLocation(t))

	want := tm.AddDate(0, 0, 1)
	got := sample.GetTomorrow(tm)
	if diff := cmp.Diff(got, want); diff != "" {
		t.Errorf("GetTomorrow() differs: (-got +want)\n%s", diff)
	}
}
//}

@<code>{testutil.GetJstLocation}を呼び出すだけになり、テストコード冒頭にあったエラーハンドリングはなくなりました。このようにテストヘルパーは、"なにをテストしているのか"について伝えやすいテストコードを作成する上で有効な働きをしてくれます。

== 外部テストパッケージ

テストコードのパッケージ名について、対象のパッケージと同一のパッケージ名にするか、外部テストパッケージを行う２つの選択肢があります。外部テストパッケージは、@<code>{xxx_test}の形式のパッケージ名になり、たとえば sample パッケージであれば sample_test というパッケージ名です。
外部テストパッケージを選択した場合、対象とは異なるパッケージになるため、パッケージの非公開機能などにはアクセスできなくなります。

===[column] 外部テストパッケージから非公開機能にアクセスする方法

外部テストパッケージからでも@<code>{export_test.go}を介してパッケージの非公開機能にアクセスする方法もあります。詳しくは、 tenntenn@<fn>{tenntenn} さんが書いた記事『Go Fridayこぼれ話：非公開（unexported）な機能を使ったテスト #golang』@<fn>{exportTestArticle}をご覧ください。

===[/column]

//footnote[tenntenn][@<href>{https://twitter.com/tenntenn}]
//footnote[exportTestArticle][@<href>{https://tech.mercari.com/entry/2018/08/08/080000}]

費用対効果の高いユニットテストを目指すために、テストパッケージはどちらの方法を選択するべきなのでしょうか。これについて知るために両者のメリット・デメリットを知る必要があります。これを考えるための材料として、ブラックボックステスト・ホワイトボックステストの２つの概念をおさえましょう。

ユニットテストは、"テスト対象の内部の動きをどれだけ知っているか"によって、ブラックボックステスト・ホワイトボックスの２種類に分類できます。ブラックボックステストは、パッケージがAPIとして公開している以外はそのパッケージについて関して何も知らないことを前提とします。対象的にホワイトボックステストはパッケージの内部機能にアクセスできます。外部テストパッケージの場合、非公開機能へのアクセスのしにくさによりブラックボックステストになりやすいでしょう。そして、同一パッケージの場合、非公開機能へのアクセスが可能になるため、ホワイトボックステストを行うことができます。

ブラックボックステストは、"壊れやすいテスト（Fragile Test）"@<fn>{fragileTest}になりにくい利点があります。壊れやすいテストとは、動作コードの変更に敏感に反応して失敗してしまうユニットテストのことをいいます。ここでいう"敏感に"とは、外部への振る舞いを変えない内部の処理詳細の変更に対してもユニットテストが失敗してしまう状況を表しています。"変更検出（change detecter）テスト"あるいは"現状維持（status quo）テスト"と呼ばれることもあります。壊れやすいテストは、必要以上の変更頻度を発生させ、ユニットテストの維持コストの増大に繋がります。ブラックボックステストを行うことは、そのような維持コストの高いユニットテストの発生を防ぐ利点があります。

//footnote[fragileTest][http://xunitpatterns.com/Fragile%20Test.html]

また、パッケージの公開機能のみを利用するユニットテストを書くため、ユニットテスト作成時にそのパッケージのクライアントの立場に立って考える機会になります。結果としてそのパッケージのAPIの使いにくい点や欠陥への気付きに繋がります。

対照的にホワイトボックステストは、内部の実装の複雑な部分を詳細に網羅する事ができる利点があります。

テストパッケージをどちらにするのかはこれらのどちらの利点を重視するかによって変わるでしょう。筆者は、壊れやすいテスト発生による維持コストの増加の懸念とパッケージのクライアントの視点の２つの利点を重視しています。そのため、ブラックボックステストのなりやすい外部テストパッケージにテストコードを置くようにしています。そのうえで、非公開機能に対して公開機能を介さずに直接ユニットテストを書きたい場合には、非公開機能を別パッケージの公開機能として切り出せないか検討します。

== おわりに

本章では、ユニットテストの費用対効果と、それを実現するためのGoのユニットテストの基礎を紹介しました。何か迷った際に、考え方・基礎を抑えておくことで判断にブレが少なくなります。これからユニットテストを書き始める方にとってはひとつの方針として、すでに書いている方にとってはこれまでのコードの振り返りの機会として本章が役立てば幸いです。
