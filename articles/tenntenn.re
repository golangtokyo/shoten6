= Goに関するよくある質問に答える

== はじめに

株式会社メルペイのバックエンドエンジニアの@tenntenn@<fn>{tenntenn}です。
筆者は、プログラミング言語Goの技術コミュニティをいくつか運営しています。
初心者向けから中級・上級者向けまでさまざまな勉強会やハンズオンを行っています。
そんな中でよく参加者に聞かれる質問がいくつかあります。
例えば、〇〇というライブラリーは使っていいのか、レシーバにはポインタを使うべきか、などです。

//footnote[tenntenn][@<href>{https://github.com/tenntenn/bio}]

そこで本章では、Goをはじめた人が1度は考えただろう疑問に対して、筆者なりの回答を示します。
ここに書かれている内容が完全に正しいとは言えませんが、多くの場合は納得いくものになっているでしょう。

== Goに入ってはGoに従え

=== プログラミング言語の文化

"Goに入ってはGoに従え"は、Go Conference 2014 Autumnのキーノート@<fn>{gocon14}で用いられた言葉です。
これは"郷に入っては郷に従え"をもじった言葉ですが、プログラミング言語を学ぶ上で非常に大切なことをうまく表しています。
タイトルだけではなく、中身も非常に勉強になるため読んだことが無い方はぜひ読むことをおすすめします。

//footnote[gocon14][@<href>{https://ukai-go-talks.appspot.com/2014/gocon.slide}]

プログラミング言語は何かしらの目的を持って人工的に開発された言語です。
開発された背景は言語によってさまざまで、研究目的で開発された言語もあれば、Goのように実用目的で開発された言語もあります。

プログラミング言語の開発された背景を知ることでその言語の文化を学ぶことができます。
"なぜこの言語には他の言語にあるこの機能が無いのか"、"なぜ文法がこうなっているのか"など言語が生まれた背景を知っていると理解が進むでしょう。

=== 公式のFAQ

Goには公式のFAQ@<fn>{gofaq}が存在しています。
もし"なぜGoには◯◯という機能が無いのだろう"という疑問が浮かんできた場合は公式のFAQに目を通すと良いでしょう。
公式のFAQには、Goの開発チームが頻繁に聞かれる疑問への回答が掲載されています。

//footnote[gofaq][@<href>{https://golang.org/doc/faq}]

本章では公式のFAQに載ってないものや、さらに一歩踏み込んだ内容を扱っています。
公式のFAQも合わせて読むとさらに理解が進むでしょう。

=== Code Review Comments

GoのWikiにはCode Review Comments@<fn>{codereviewcomments}というドキュメントがあり、そこにはGoのコードをレビューする際に着目するポイントが記述されています。
有志によって日本語訳もされています@<fn>{codereviewcomments_ja}。

//footnote[codereviewcomments][@<href>{https://github.com/golang/go/wiki/CodeReviewComments}]
//footnote[codereviewcomments_ja][@<href>{https://qiita.com/knsh14/items/8b73b31822c109d4c497}]

変数名の付け方やレシーバをポインタにすべきなど、初学者が迷う点も記載されてるいるため、Goをひと通り学んだ後に一度目を通すと良いでしょう。
初学者が最初のレビューでGoの熟練者に受ける指摘の多くはここに掲載されているはずです。

== ◯◯というライブラリを使うべきか

=== ライブラリの導入で気をつけること

筆者は"Goで◯◯というライブラリを使うべきか"という議論をよく目にします。
ライブラリは便利な機能を提供し、車輪の再開発を防ぐことができます。
しかし、ライブラリの選定を間違えると、複雑さが増し自分たちのコードも保守しづらくなります。

ライブラリの選定は開発チームで方針を決め、コードレビューでレビューアーがしっかりとチェックすべきです。
ライブラリの選定基準としては次のような項目がよく用いられるでしょう。

 * 有名なライブラリか（GitHubのスター数が多い）
 * メンテナンスされているか
 * ライセンスがしっかりしているか

有名でかつメンテナンスがしっかりされて入れば、導入しても問題なさそうです。
しかし、筆者はそれだけでは不十分だと考えています。
ライブラリを導入する際にかかるコストや増加する複雑さと向上する利便性を天秤にかける必要があります。

ライブラリを導入するとチームではコントロールできない外的要因が増えます。
導入時にメンテナンスがされていても今後のGoのバージョンアップに追従するとは限りません。
ライブラリのバージョンアップによって互換性が途絶えてしまい、マイグレーションにコストがかかかってしまうかもしれません。

また、ライブラリの一部を使う前提で導入しても将来的に想定外の使い方をされてしまう恐れがあります。
ライブラリが提供する機能が多い場合には特定のパッケージのみに使用を制限し、
他のパッケージを使っている場合は静的解析ツールなどで機械的にチェックできるようにしておくと良いでしょう。

ライブラリの導入によってチームメンバーの学習コストが増加することも意識すべきです。
新しく開発チームに入ってくるメンバーはGoについての知識はあるはずですが、ライブラリの知識を持っているとは限りません。
多機能で複雑なライブラリを導入することは、学習コストをぐっと上げてしまう恐れがあります。

どんなライブラリであってもこのような視点を持ってチームメンバーの同意が取れれば導入しても問題ないでしょう。
重要なのは向上する利便性と増加する複雑さのバランスです。
できるだけ必要最低限の機能のみを提供するライブラリを選定するように心がけましょう。
また、ライブラリを作る際はあまり多機能なライブラリを作らずコンパクトにまとめるようにすると利用されやすいのではないでしょうか。

=== 標準ライブラリを使おう

サードパーティ製のライブラリを利用する前に、標準ライブラリで実現可能ではないか確認するようにしましょう。
標準ライブラリは基本的にはGoのバージョンアップに追従していくため、急に利用ができなくなったりすることはありません。
そのため、標準ライブラリを積極的に使うようにし、ちょっとした機能であれば標準パッケージだけで自作してしまった方が複雑さの増加を防ぐことに繋がります。

標準ライブラリにほしい機能がない場合、golang.org/x以下で管理されている準標準ライブラリの利用を考えましょう。
準標準ライブラリには、標準ライブラリにはない機能が用意されていたり、将来的に標準ライブラリに昇格するようなパッケージがあります。
準標準ライブラリもGoのコアチームによって管理されているため、比較的Goのバージョンアップに追従しています。

=== testifyは使うべきか

筆者はtestifyというテスティングフレームワークを使うべきかという質問をよく受けます。
testifyは非常に有名なライブラリで著名なOSSでも使われています。
そのため、利用しても問題なさそうですが、上述の通り、複雑さが増加することを考慮する必要があります。

testifyを使う理由は、おそらく@<code>{testify/assert}パッケージが利用したいからでしょう。
@<code>{assert}パッケージを利用することで@<list>{assert}のようにチェックしたい値を比較し、簡単にエラーメッセージを生成することができます。

//list[assert][assertパッケージの利用][go]{
import (
  "testing"
  "github.com/stretchr/testify/assert"
)

func TestSomething(t *testing.T) {

  var a string = "Hello"
  var b string = "Hello"

  assert.Equal(t, a, b)

}
//}

testifyには@<code>{assert}パッケージ以外にも@<code>{suite}パッケージなどがあります。
@<code>{suite}パッケージは、Go 1.7以前では便利でしたがGo1.7からサブテストの機能が@<code>{testing}パッケージに入ったため、
使用するメリットが減りました。

@<code>{assert}パッケージだけを利用するのであれば、そこまで複雑さは増えないでしょう。
しかし、将来的にそのルールが維持されるかどうかの保証はないため、慎重に選択するべきです。

@<code>{assert}パッケージの利用も公式のFAQにあるように、テストのためのミニ言語を作らないという思想からは外れてしまいます。
@<code>{github.com/google/go-cmp}パッケージなど、比較の機能だけを提供するライブラリの方が影響範囲が小さく、ライブラリの導入コストは低いでしょう。
リッチなアサーションの機能が本当に必要か改めて考え、@<code>{go-cmp}パッケージや@<code>{reflect.DeepEqual}関数で代用できないか検討しましょう。

== ポインタは使うべきか

=== レシーバはポインタにする

Code Review Commentsにあるように@<fn>{receiver_type}、レシーバは基本的にはポインタにすべきです。
レシーバは引数と同様にコピーされてメソッドに渡されます。
@<list>{receiver}のようにレシーバをポインタにしない場合、フィールドの値を変えようとして失敗してしまいます。
レシーバがポインタではないため、コピーされてメソッドに渡された構造体のフィールドを変えるだけで元の構造体のフィールドの値は変わりません。

//footnote[receiver_type][@<href>{https://github.com/golang/go/wiki/CodeReviewComments#receiver-type}]

//list[receiver][レシーバがポインタではない場合][go]{
package main

import "fmt"

type T struct {
  N int
}

func (t T) SetN(n int) {
  t.N = n // tはコピーされた値
}

func main() {
  var t T
  t.SetN(100)
  // 0
  fmt.Println(t.N)
}
//}

@<list>{pointer_receiver}のようにレシーバをポインタにした場合、
@<code>{t.M()}と記述しても@<code>{(&t).M()}と記述したように扱ってくれます。
このシンタックスシュガーがあるため、レシーバをポインタにしても利便性が落ちることはあまりありません。

//list[pointer_receiver][レシーバがポインタの場合][go]{
type T int
func (t *T) M() {
    fmt.Println("method")
}

func main() {
    var t T
    t.M() // (&t).M()と同じ
}
//}

=== スライスの要素はポインタにする

レシーバをポインタにした場合、スライスの要素もポインタにすべきです。
その理由の1つとして@<list>{slice_interface}のようにスライスの要素をインタフェースとして引数に渡す際に面倒だからです。
レシーバが@<code>{*T}型であるため、インタフェースを実装しているのは@<code>{*T}型になります。
そのため、スライスの要素の型が@<code>{T}型である場合、そのポインタを取る必要がでてきます。
はじめから要素にポインタを使用していれば、このような記述は必要ないでしょう。

//list[slice_interface][スライスの要素とインタフェース][go]{
type T struct{S string}
func (t *T) String() string { return t.S }
var _ fmt.Stringer = (*T)(nil)

func f(s fmt.Stringer) {    
    fmt.Println(s.String())
}

func g(ts []T) {
    for i := range ts {
        f(&ts[i])
    }
}
//}

また、@<list>{slice_loop}のようなコードを書いた場合に、そこにバグが含まれていることが気づきづらいでしょう。
@<code>{ts}の要素が@<code>{T}型の場合、@<code>{for}文の変数@<code>{t}には要素のコピーが代入されます。
そのため、いくら@<code>{for}の中で変数@<code>{t}のフィールドを更新してもスライスの要素には何の影響も与えません。

//list[slice_loop][for rangeで起きるコピー][go]{
// tsは[]T型のスライス
for i, t := range ts {
     // tはコピーされるため、ここでフィールドを変更しても意味がない    
     t.S = strings.Repeat("*", i+1)
}
//}

一方、スライスの要素をポインタにした場合、@<code>{for}文の中でコピーされる要素の値はポインタです。
そのため、フィールドへの更新はポインタを通じてスライスの要素に影響を与えます。

このように、スライスの要素をポインタにすることで未然にバグを防ぐことに繋がります。
スライス（その背後にある配列）はメモリ上に要素が並んで確保されていることが重要であり、
その要素をポインタにするとパフォーマンスが悪くなる可能性があります。
しかし、多くの場合はパフォーマンスへの影響よりも見つけづらいバグを防ぐ効果の方が高く、
パフォーマンスの問題に対処するには実際に計測して問題が見つかってからでも遅くありません。
そのため、スライスの要素はメソッドのレシーバに合わせて、特に理由がない場合はポインタにする方が無難でしょう。

== New◯◯関数の戻り値をインタフェースにすべきか

=== インタフェースとそれを実装する具象型の提供

とあるパッケージが@<list>{client_interface}のようなインタフェース型を提供したい場合を考えます。

//list[client_interface][パッケージが提供したいインタフェース][go]{
type Client interface {    
  Do() error
}
//}

このインタフェースは@<list>{arg_client}のように関数の引数に用いられることを想定しています。

//list[arg_client][Clientインタフェースを引数に取る関数][go]{
func SendRequest(ctx context.Context, cli Client) error {
  // (略)
}
//}

ここで、このインタフェースを実装する具象型の提供方法を考えます。
例えば、@<list>{new_conreate}のように、@<code>{MyClient}型という@<code>{Client}インタフェースを実装する型を作り、
@<code>{New}関数の戻り値で返すような実装があるでしょう。

//list[new_conreate][具象型を返すNew関数][go]{
type MyClient struct{}
func (c *MyClient) Do() error { return nil }

// Newは具象型の*MyClientを返す
func New() *MyClient {
  return &MyClient{}
}
//}

しかし、@<code>{New}具象型を返してしますといくつか問題があります。
次に具象型を返す場合の問題点を紹介し、戻り値をインタフェース型にする場合を考えてみましょう。

=== 戻り値をインタフェース型にする

@<code>{New}関数が具象型で返す場合、次のような問題があります。

 * 利用者側の視点からみるとインタフェース型と具象型の2つの型を知る必要ある
 * 具象型が持つインタフェースにはない機能に依存される可能性がある
 * 実装を変えることが不可能になる

このパッケージでは、インタフェース型の提供を主目的としているため、具象型の提供はそもそも必要としてしていません。
そのため、デフォルトの実装となる具象型をパッケージ外部から見える形で提供してしまうと、そこに不要な依存関係が発生したり、
パッケージ利用者の関心事がインタフェース型と具象型で複数に増えることになってしまいます。

そこで@<list>{new_interface}のように具象型はエクスポートせずに、@<code>{New}関数の戻り値をインタフェース型にすることで解決します。

//list[new_interface][インタフェース型を返すNew関数][go]{
// 具象型はエクスポートしない
type client struct{}
func (c *client) Do() error { return nil }

// Newはインタフェース型のClientを返す
func New() Client {
  return &client{}
}
//}

パッケージ利用者の関心事は@<code>{Client}インタフェースに限定され、具象型への不要な依存はなくなります。

=== インタフェースは利用者側で提供する

Code Review Commentsにはインタフェースは利用者側で提供すべきと書かれています。
つまり、実際にインタフェースを引数で受け取る関数やメソッドを提供しているパッケージがインタフェースを定義しなければならないということです。

また、モックのためにインタフェースを設けるべきではなく、実際の実装でテストできるような設計にすべきとも書かれています。
つまり、インタフェースを利用するシーンを想定せずに、テストがしやすいという理由でインタフェースを定義してはならないということでしょう。

前述の@<code>{Client}インタフェースの例に戻ると、Code Review Commentsの指摘と矛盾しているように見えます。
しかし、この例ではCode Review Commentsに記述されているシチュエーションとは異なり、具象型の提供者とインタフェースの提供者の両方の立場をとっています。
具体的には、@<code>{SendRequest}関数のようにインタフェースを引数として受ける関数の提供（インタフェースの利用者）と
@<code>{New}関数でデフォルトの実装の提供（具象型の提供者）を行っています。
このように、インタフェースの利用者側と実装の提供者側の両方の立場を取る場合は、筆者は具象型をエクスポートすべきではないと考えています。

== おわりに

本章では筆者がよく質問されるGoの疑問について紹介し、筆者の考えを織り交ぜながら回答しました。
ここで挙げた疑問以外にも"モックはどうするべきか"や"パッケージ構成はどうするべきか"など一筋縄にはいかないような疑問もあります。
こうした話はぜひ読者のみなさんとgolang.tokyoで議論できたら幸いです。