= Goによる画像処理&画像解析

== はじめに

こんにちは。白ヤギコーポレーションでバックエンドエンジニアをしているpo3rinです。この章ではGoによる画像処理&画像解析の方法を紹介します。最初は入門から始まり、中盤からは様々な画像処理のTipsを紹介します。終盤では更に突っ込んでOpenCVを使った顔認識や輪郭抽出などの方法を紹介します。初めての方でもこの章を読み終わる頃にはGoでエレガントに画像を扱えるようになっていることでしょう。ソースコードは全て私のGitHubリポジトリ@<fn>{repo}に上げているので参考にどうぞ。
//footnote[repo][https://github.com/po3rin/go-image-manipulation]

=== 今回使う素材画像について

今回、@tottieさんのGopher@<fn>{gopher}くんのイラストの利用許可を頂きました。@<img>{po3rin-tottie-gopher}など素敵なイラストを描いている方です。是非Twitterなどでフォローしてみて下さい。
//footnote[gopher][Gopherの原作者は Renée French さんです]

//image[po3rin-tottie-gopher][@tottieさんの可愛いGopherくんの画像][scale=0.4]{
//}

また、今回の章で使用するダウンロードデータを皆さんが金銭の発生する二次利用として利用してしまうと法律的にアウトなのでご了承ください。

== imageパッケージの基本を抑えよう

まずはGoで画像を扱う際の中心になるimageパッケージの基本を押さえましょう。このタイミングで@tottieさんのGopherくんのイラスト(@<img>{po3rin-tottie-gopher})をダウンロードしておいてください。

//list[download_image][可愛いGopherくんの画像をローカルに落とす][]{
$ curl -o gopher.png \
https://s3-ap-northeast-1.amazonaws.com/po3rin-golangtokyo/img/gopher.png
//}

=== image.Imageの基本

まずは画像ファイルを@<code>{image.Image}の型に変換して画像のデータをみてみましょう。

//list[getimg][画像ファイルをimage.Imageに変換][go]{
package main

import (
	"fmt"
	"image/color"
	"image/png"
	"log"
	"os"
)

func main() {
	img, _ := png.Decode(os.Stdin)

	// カラーモードがNRGBAか確認。
	// output : true
	fmt.Println(img.ColorModel() == color.NRGBAModel)

	// 画像の境界を所得。値は一番左上を(0,0)とした時の(x,y)。単位はpx。
	// output : (0,0)-(460,460)
	fmt.Println(img.Bounds())

	// 指定したピクセルの画素値を返します。
	// output : {255 255 255 255}
	fmt.Println(img.At(0, 0))
}
//}

@<code>{png.Decode}で標準入力を@<code>{image.Image}型にデコードしています。@<code>{Image.At}メソッドの引数や@<code>{Image.Bounds}メソッドで返ってくる画像の座標は一番左上を@<m>{\left( 0,0\right)}としています。これを実行すると画像の縦横の長さや、色についての情報が取れます。

//list[rungo][main.goの実行][]{
$ go run main.go < gopher.png
//}

===[column] カラーモードとアルファチャネルについて

RGBAとは、ディスプレイ画面で色を表現するために用いられる、赤（Red）、緑（Green）、青（Blue）の3色に、アルファ（Alpha）と呼ばれる透過度の情報を加えたもののです。RGBチャンネルにアルファ値が乗算されている形式をPre-multiplied Alpha(乗算済みアルファ)、乗算されていない形式をStraight Alphaと呼びます。Goのimage/colorパッケージでは、NRGBAはStraight Alphaを表現しています。

===[/column]

@<code>{image.Image}の実装をみてみましょう。今回使った3つのメソッドを持つインターフェースになっています。

//list[image_Image][image.Imageの実装][go]{
type Image interface {
	ColorModel() color.Model    // 画像のカラーモードを返す。
	Bounds() Rectangle          // 画像の領域を返す。
	At(x, y int) color.Color    // 指定座標のカラーを返す。
}
//}

一つ注意点として、Goではpng,jpeg,gifのイメージフォーマットが標準パッケージでサポートされています。これらを使う際にはパッケージのメソッド等を使わない場合でも@<code>{import _ image/png}のようにブランクインポートで対応するパッケージを読み込んでおかないと実行時にエラーが発生します。理由を探る為に@<code>{image/png/reader.go}をみてみましょう。

//list[image_register][image.RegisterFormatによる画像フォーマット登録][go]{
func init() {
	image.RegisterFormat("png", pngHeader, Decode, DecodeConfig)
}
//}

上のようにinit関数で使える画像フォーマットをパッケージ内のinit関数内で登録しています。その為、init関数を発火させる為にブランクインポートをする必要があります。

=== 画像のカラーモードを変える

さて、image.Imageの基礎を抑えたところでimageパッケージでよくある実装パターンを紹介します。例として画像をグレースケールに変換する処理をみてみましょう。

//list[grayscale][画像をグレースケールに変換][go]{
func main() {
	img, _ := png.Decode(os.Stdin)
	bounds := img.Bounds() // (0,0)-(460,460)

	// 受け取った画像と同じ大きさのカラーモードGray16の画像を生成.
	// この時点では460*460の真っ黒の画像です。
	dst := image.NewGray16(bounds)

	// 1ピクセルずつ処理します。
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			// 元画像の(x,y)ピクセルのカラーをGray16に変換。
			c := color.Gray16Model.Convert(img.At(x, y))
			gray, _ := c.(color.Gray16)

			// 先ほど作ったdstに先ほど変換したカラーをセット。
			dst.Set(x, y, gray)
		}
	}
	png.Encode(os.Stdout, dst)
}
//}

これを実行するとグレイスケールのGopherくん(@<img>{po3rin-gray_gopher}）ができます。

//list[withoutput][main.goの実行][]{
$ go run main.go < gopher.png > gray_gopher.png
//}

//image[po3rin-gray_gopher][可愛いGopherくんのグレー画像][scale=0.4]{
//}

先ほどのソースコードの通り、Goで画像加工をする際には、まず目的の画像(dstと呼ぶ)を作成し、元画像(srcと呼ぶ)から1画素ずつ処理してdstにセットするという流れになります。濃淡加工やセピア加工などのエフェクト処理においてもこのフローは鉄板なので覚えておきましょう。

== 線形補間法を実装して画像リサイズの仕組みを学ぼう

今までの知識を使って少し骨のある実装をしてみましょう。@<code>{resize}パッケージを作り、画像を指定倍率でリサイズできる@<code>{Resize}関数を実装します。今回は標準パッケージのみで実装していく事で、Goにおける実装方法だけでなく、画像の拡大縮小の仕組みも学んでいきましょう。

=== 線形補間法(lerp)の概要と実装

単純に画像の拡大縮小を考えると何が起きるのでしょうか。単純に拡大すると元々の色はそのままで風船が膨らむように拡大されます。そうなると拡大後には@<img>{po3rin-resize}のように色の無い隙間ができるはずです。

//image[po3rin-resize][画像の拡大縮小は隙間が発生する][scale=0.4]{
//}

その為、この隙間を何かしらの計算アルゴリズムで補間する必要があります。ここで紹介するのが線形補間法です。線形補間法はコンピュータグラフィックスを含む多くの分野で非常によく使われているシンプルな補間の1つで、計算量が非常に少ない事が特徴です。拡大後の画素値は、元画像の位置から最も近い画素(近傍4画素と呼ぶ)からの距離によって計算する方法です。@<img>{po3rin-lerp}は線形補正法の概念図と計算式です。

//image[po3rin-lerp][線形補正法の概念図と計算式][scale=1]{
//}

#@# //texequation{
#@# f\left( x',y'\right) = f\left( x,y\right) \alpha \beta + f\left( x+1,y\right) \left( 1-\alpha\right) \beta + f\left( x,y+1\right) \alpha \left( 1-\beta\right) + f\left( x+1,y+1\right) \left( 1-\alpha\right) \left( 1-\beta\right)
#@# //}

@<m>{\left( x',y'\right)}の周りの点は先ほど説明した近傍4画素です。今回のケースにおける関数@<m>{f}は拡大縮小後の座標を渡せば補正後の画素値を返してくれる関数です。@<m>{\alpha}と@<m>{\beta}は座標間の距離を表します(座標ごとの重み付けのため)。やはりエンジニアたるもの、コードを書いてこそ理解できるというもの。早速lerpパッケージを作成しましょう。このパッケージは画像に関するコードは排除し、単純にLerpのアルゴリズムを実装したパッケージです。

//list[resize-lerp][lerpパッケージの実装][go]{
package lerp

// Point lerpで使う座標
type Point struct {
	X int
	Y int
}

// Points lerpで使う近傍4座標
type Points [4]Point

// PosDependFunc 関数f
type PosDependFunc func(x, y int) float64

// Lerp calicurate relp
func Lerp(f PosDependFunc, a float64, b float64, ps Points) float64 {
	n := (1.0-b)*(1.0-a)*f(ps[0].X, ps[0].Y) +
		a*(1.0-b)*f(ps[1].X, ps[0].Y) +
		b*(1-a)*f(ps[0].X, ps[1].Y) +
		a*b*f(ps[1].X, ps[1].Y)
	return n
}
//}

今回はコードを追いやすいようにシンプルな形でパッケージを作りました。Lerpの式はLerp関数で実装しています。引数に関数@<m>{f}、@<m>{\alpha}、@<m>{\beta}、近傍4画素の位置だけ渡せば実行できるのがわかります。

=== 線形補間を使った画像リサイズ

lerpパッケージを使う為に新しい関数を2つ作成します。1つ目は近傍点の座標と@<m>{\alpha}や@<m>{\beta}などのパラメータを計算する関数。もう一つはRGBAのいずれか1つを抽出する関数@<m>{f}を返す関数です。なぜ2つ目の関数が必要かというと、Lerpは今回RGBAそれぞれの値に対して実行します。つまり。計4回Lerpを行うので関数@<m>{f}を4つ作る必要があります。initGetOneColorFuncは4つの関数@<m>{f}を生成するプロセスを一つにまとめただけです。

//list[innnerfunc][画像リサイズに使う関数][go]{
// getLerpParam 1軸に対して、Lerpで使うパラメータと近傍点をdstの座標と拡大縮小比率から所得する関数
func getLerpParam(dstPos int, ratio float64) (int, int, float64) {
	// 拡大前の座標の所得 (拡大後の座標 / リサイズ比率)
	v1float := float64(dstPos) / ratio

	// 拡大前の座標から最も近い2つの整数値を所得
	v1 := int(v1float)
	v2 := v1 + 1

	// (拡大前の座標の浮動小数点数 - 拡大前の座標の整数値)
	v3 := v1float - float64(v1)
	return v1, v2, v3
}

// initGetOneColorFunc srcのRGBAからいずれか1つを抽出する関数fを返す関数。
func initGetOneColorFunc(src image.Image, colorName string) lerp.PosDependFunc {
	return func(x, y int) float64 {
		var c uint32
		switch colorName {
		case "R":
			c, _, _, _ = src.At(x, y).RGBA()
		case "G":
			_, c, _, _ = src.At(x, y).RGBA()
		case "B":
			_, _, c, _ = src.At(x, y).RGBA()
		case "A":
			_, _, _, c = src.At(x, y).RGBA()
		}
		return float64(c)
	}
}
//}

それではいよいよ本番。先ほど作った関数と自作のlerpパッケージを使い、受け取った @<m>{\left( x,y\right)}に対して線形補正して画素値を返すエフェクタを作りましょう。

//list[lerp][線形補正法を使った画素値の計算][go]{
// LerpEffect (x,y)に対してLerpを行った結果の画素値を返す関数
// src は元画像 x,y はdstのx,yです。
func LerpEffect(src image.Image, xRatio, yRatio float64, x, y int) color.RGBA64 {
	// 元画像の近傍４画素の座標と、alpha、betaを所得
	x1, x2, alpha := getLerpParam(x, xRatio)
	y1, y2, beta := getLerpParam(y, yRatio)

	// 元画像の近傍４画素の座標を格納
	ps := lerp.Points{
		{X: x1, Y: y1},
		{X: x2, Y: y1},
		{X: x1, Y: y2},
		{X: x2, Y: y2},
	}

	// RGBAそれぞれの値に対してLerpを適用
	r := lerp.Lerp(initGetOneColorFunc(src, "R"), alpha, beta, ps)
	g := lerp.Lerp(initGetOneColorFunc(src, "G"), alpha, beta, ps)
	b := lerp.Lerp(initGetOneColorFunc(src, "B"), alpha, beta, ps)
	a := lerp.Lerp(initGetOneColorFunc(src, "A"), alpha, beta, ps)

	return color.RGBA64{uint16(r), uint16(g), uint16(b), uint16(a)}
}
}
//}

ではいよいよ終盤、グレースケールでやった方法と同じように、1ピクセルずつ今回作ったエフェクターで処理し、dstにセットして拡大縮小後の画像を完成させます。

//list[resize][Resize関数の完成][go]{
//Resize は与えられた画像を線形補間法を使用して画像を拡大・縮小する。
func Resize(img image.Image, xRatio, yRatio float64) image.Image {
	// 拡大後のサイズを計算
	width := int(float64(img.Bounds().Size().X) * xRatio)
	height := int(float64(img.Bounds().Size().Y) * yRatio)

	// 元となる新しい拡大画像(dst)を生成する
	newRect := image.Rect(0, 0, width, height)
	dst := image.NewRGBA64(newRect)
	bounds := dst.Bounds()

	// 1画素ずつ線形補正法を使って画素値を計算してdstにセット
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			dst.Set(x, y, LerpEffect(img, xRatio, yRatio, x, y))
		}
	}
	return dst
}
//}

おめでとうございます！標準パッケージだけで画像リサイズ関数を作る事ができました！

== 画像やテキストを合成してOGP画像を生成してみよう

ここからは更に実践的な内容に入りましょう。ブログを作成する際にOGP画像を作成するのってメンドくさいですよね。この節では画像やテキストの合成をGoで行い、OGP画像を自動で生成できるようにしましょう。今回は@<img>{po3rin-ogp}のようなOGP画像生成を目指します。

//image[po3rin-ogp][今回作るOGP画像][scale=0.6]{
//}

=== 画像の合成

今回、画像の合成に追いては@<img>{po3rin-synthesis-flow}のようなフローを踏みます。

//image[po3rin-synthesis-flow][dstに画像を合成のフロー][scale=0.8]{
//}

まずは今回必要な画像を準備しましょう。

//list[download_image][OGPの素材画像を準備][]{
$ curl -o cover.jpg \
https://s3-ap-northeast-1.amazonaws.com/po3rin-golangtokyo/img/cover.jpg

$ curl -o go.png \
https://s3-ap-northeast-1.amazonaws.com/po3rin-golangtokyo/img/go.png
//}

では必要な画像を準備してくれる関数を作りましょう。必要なのはsrc(Goのロゴ)とcover(背景画像)とmask(背景画像をマスキングする画像)とdst(合成を受ける画像)です。画像はそれぞれOGP画像の大きさに合うように前節で実装したリサイズ関数を使いましょう。前節の実装を飛ばした方は、@<code>{github.com/po3rin/resize}パッケージとして全く同じリサイズ機能を提供するパッケージを用意したのそちらを使いましょう。

//list[getimagefunc][必要な画像を返す関数群][go]{
// NewRect 指定色で塗りつぶした画像を生成する。
func NewRect(r image.Rectangle, c color.Color) draw.Image {
	dst := image.NewRGBA(r)
	rect := dst.Rect
	for h := rect.Min.Y; h < rect.Max.Y; h++ {
		for v := rect.Min.X; v < rect.Max.X; v++ {
			dst.Set(v, h, c)
		}
	}
	return dst
}

// GetSrc 合成する画像を読み込んで横幅300の画像にリサイズした結果を返す。
func GetSrc() image.Image {
	src, _, _ := image.Decode(os.Stdin)
	rate := float64(300) / float64(src.Bounds().Max.X)
	src = Resize(src, rate, rate)
	return src
}

// GetCover カバー画像を読み込み、OGP画像(横幅1200px)に合うようにリサイズした結果を返す。
func GetCover() image.Image {
	f, _ := os.Open("./cover.jpg")
	defer f.Close()
	cover, _, _ := image.Decode(f)
	rate := float64(1200) / float64(cover.Bounds().Max.X)
	cover = resize.Resize(cover, rate, rate)
	return cover
}
//}

@<code>{NewRect}はdstとmaskを作る為の関数です。新しいRGBA画像を作成して、1画素ごとに引数で受け取った色をセットしているだけです。そして、GetCoverとGetSrcはそれぞれ合成する画像を読み込んでOGP画像の大きさに合うようにリサイズしています。これらの関数を使って画像合成をしてみましょう。

//list[synthesis][画像の合成][go]{

func main() {
	r := image.Rectangle{Min: image.Point{0, 0}, Max: image.Point{1200, 630}}

	dst := NewRect(r, color.RGBA{0, 0, 0, 250})
	src := GetSrc()
	cover := GetCover()
	mask := NewRect(r, color.RGBA{0, 0, 0, 140})

	// coverをmaskでマスキングした結果をdstに合成
	draw.DrawMask(dst, r, cover, image.Pt(0, 0),
		mask, image.Pt(0, 0), draw.Over,
	)
	// srcを真ん中に合成。
	draw.Draw(dst, r, src,
		image.Pt(
			-r.Bounds().Max.X/2+src.Bounds().Max.X/2,
			-r.Bounds().Max.Y/2+src.Bounds().Max.Y/2,
		),
		draw.Over,
	)

	// 書き出し
	png.Encode(os.Stdout, dst)
}
//}

ここで使っている@<code>{image/draw}パッケージは画像合成機能を提供しています。@<code>{draw.Draw}の実装を見ると単に@<code>{draw.DrawMask}をラップした関数になっています。

//list[draw][draw.Drawの実装][go]{
// Draw calls DrawMask with a nil mask.
func Draw(dst Image, r image.Rectangle, src image.Image, sp image.Point, op Op) {
	DrawMask(dst, r, src, sp, nil, image.Point{}, op)
}
//}

では@<code>{draw.DrawMask}の引数をみていきましょう。

//list[drawmask][draw.DrawMaskの実装][go]{
// 長い為改行
func DrawMask(
	dst Image, r image.Rectangle, src image.Image, sp image.Point,
	mask image.Image, mp image.Point, op Op
)
//}

@<code>{draw.DrawMask}は、srcをmaskでマスキングしてdst内に配置します。srcの中で合成に使う位置がsp、maskの中でマスキングに使う位置がmpです。opはPorter-Duffコンポジションの演算子で、２枚の画像を合成する際にそれぞれのピクセルについてどのように処理するかを規定したものです。Goでは二つの演算子を提供しており、今回はOver演算子を使って指定されたdstの上にastを重ねています。もう1つのSrc演算子は、dstの内容に関係なく完全に上書きします(op=draw.Srcにして実行してみると結果が変わります)。基本的にSrc演算子の方が高速なので、合成の際にはSrc演算子が使えないか検討しましょう。

===[column] src,mask,dstの意味とそれぞれのPointについて

これまでdstやsrcという名前を使ってきましたが、それぞれどういう意味なのでしょうか。dstはdestination(目的地)という意味でsrcはソースという意味ですね。つまり目的地対してソースを使って処理をするというのが基本の考え方です。それに対してmaskはsrcに対してマスキングを行う為の画像を意味します。draw系関数のそれぞれのPointは@<img>{po3rin-mask}のようにrの開始ポイントを指定しています。

//image[po3rin-mask][src,mask,dstを使ったDrawMaskの考え方(Go公式ブログから引用)][scale=0.8]{
//}

===[/column]

drawパッケージの解説と実際の使い方を抑えました。次はテキストの合成です。

=== テキストの合成

テキストの合成においては準標準パッケージも使っていきます。テキストをdstに合成する関数を作りましょう。

//list[text][画像にテキストを合成する関数の実装][go]{
// DrawText テキストの合成
package main

import (
	// 省略...

	"github.com/golang/freetype"
	"golang.org/x/image/font"
)

func DrawText(img draw.Image, text string) image.Image {
	// フォントファイルを読み込んでfreetype.Fontにパース
	file, _ := os.Open("./mplus-1c-regular.ttf")
	fontBytes, _ := ioutil.ReadAll(file)
	f, _ := freetype.ParseFont(fontBytes)

	// freetypeの機能で画像にテキストを合成
	if f != nil {
		c := freetype.NewContext()
		c.SetFont(f)
		c.SetFontSize(38)
		c.SetClip(img.Bounds())
		c.SetDst(img)
		c.SetSrc(NewRect(img.Bounds(), color.RGBA{255, 255, 255, 255}))
		c.SetHinting(font.HintingNone)
		pt := freetype.Pt(300, 500)
		_, _ = c.DrawString(text, pt)
	}
	return img
}
//}

分かりにくいのは@<code>{freetype.Context.SetSrc}です。なぜここで真っ白な画像を渡しているのでしょうか。@<code>{freetype.Context.DrawString}の実装を見ると、内部で@<code>{draw.DrawMask}が実行されています。ここではsrcに対しテキストの形のマスクをかけています。その為、srcの色がそのままテキストの色になっているのです。それでは先ほど画像合成をしていたmain関数にこの関数を差し込みましょう。

//list[textdraw][テキストの合成][go]{
func main() {
	// 省略...

	// 合成するテキストを定義してDrawText関数に渡す
	text := "Goではじめる画像処理、画像解析"
	d := DrawText(dst, text)

	// 書き出し
	png.Encode(os.Stdout, dst)
}
//}

お疲れ様です。、実行の準備が整いました。早速OGP画像を生成してみましょう。

//list[download_image][OGP生成を実行][]{
$ go run main.go < go.png > result.png
//}

これでOGP画像を生成するコードができました！テキストとsrc画像を標準入力やフラッグで渡せるようにすれば、いろんなブログのOGPがすぐに作れそうです。

== OpenCVを使って画像解析をやってみよう

ここからはOpenCVを駆使して画像に対して更に進んだ処理を行ってみましょう。

=== OpenCVとは

画像処理・画像解析および機械学習等の機能を持つライブラリです。C/C++、Java、Python、MATLAB用ライブラリですが、これをGo言語で扱うためにラップしたパッケージGoCVがありますのでこちらを使います。2018年11月にOpenCV 4.0がリリースされ、GoCVも4.0に対応しています。

=== OpenCVの環境構築

OpenCVを使うためには環境構築が必要です。インストール手順がOSによって違っていたりするので今回はDockerを使いましょう。とりあえず OpenCVが使えることを確かめるコードを準備しておきます。

//list[opencvver][OpenCVとGoCVのバージョンの表示][go]{
package main

import (
	"fmt"

	"gocv.io/x/gocv"
)

func main() {
	fmt.Printf("gocv version: %s\n", gocv.Version())
	fmt.Printf("opencv verson: %s\n", gocv.OpenCVVersion())
}
//}

そしてDockerfileを準備してビルドします。

//list[dockerfile][GoCVを使う為のDockerfile][]{
FROM denismakogon/gocv-alpine:4.0.1-buildstage as build-stage
RUN go get -u -d gocv.io/x/gocv
//}

//list[build-dockerfile][Dockerfileのビルド][]{
$ docker build ./ -t gocv-playground
//}

毎回@<code>{docker run}に引数指定していくのも辛いのでMakefileを作っておきましょう。

//list[makefile][Makefile][Makefileでdocker runを簡単にできるようにしておく]{
PHONY: run
run:
	docker run --rm -v ${PWD}:/go/src/gocv-playground \
		gocv-playground /bin/sh -c "${CMD}"
//}

こうすると下記で任意のコマンドが実行できるようになります。それぞれgocvのバージョンとOpenCVのバージョンです。これでGoでOpenCVを使う環境が整いました。

//list[run-docker-in-makefile][Dockerfile][makeでコマンド実行]{
$ make run CMD="go run main.go"
gocv version: 0.18.0
opencv verson: 4.0.1
//}

=== 顔認識

OpenCVで鉄板の顔認識をやってみます。画像を読み込んで顔認識した地点を青い四角で囲みましょう。今回のサンプルは正面の顔を対象にしています。まずは必要な物を揃えましょう。1つ目は人間数人が正面で写った画像。もう1つは学習済みカスケード型分類器データ（カスケードファイル）です。今回の要件にあったhaarcascade_frontalface_alt.xml@<fn>{casc}をダウンロードしておきます。
//footnote[casc][カスケードファイルはOpenCVのリポジトリ(https://github.com/opencv/opencv/tree/master/data/haarcascades)からもダウンロードできます]

//list[download_image][顔認識用に必要なファイルをダウンロード][]{
$ curl -o haarcascade_frontalface_alt.xml \
https://s3-ap-northeast-1.amazonaws.com/po3rin-golangtokyo/casc/haarcascade_frontalface_alt.xml

$ curl -o nogi.jpg \
https://s3-ap-northeast-1.amazonaws.com/po3rin-golangtokyo/img/nogi.jpg
//}

これで準備ができました。gocvパッケージを使ってコードを書いていきましょう。

//list[face-detect][顔認識][go]{
package main

import (
	"fmt"
	"image/color"

	"gocv.io/x/gocv"
)

func main() {

	// classifier の初期化
	classifier := gocv.NewCascadeClassifier()
	defer classifier.Close()
	if !classifier.Load("./haarcascade_frontalface_alt.xml") {
		log.Fatal("Error reading cascade file")
	}

	// 画像パスを指定してMat形式に
	img := gocv.IMRead("./nogi.jpg", gocv.IMReadColor)
	if img.Empty() {
		log.Fatal("Error reading image")
	}

	// 顔検知
	rects := classifier.DetectMultiScale(img)
	fmt.Printf("found %d faces\n", len(rects))

	// 囲むための色を設定(今回は青)
	blue := color.RGBA{0, 0, 255, 0}

	// 認識した顔の数だけ四角で囲む
	for _, r := range rects {
		// 引数は (Mat形式のデータ、範囲、色、線の太さ)
		gocv.Rectangle(&img, r, blue, 3)
	}

	// 結果を画像に書き出しΩ
	gocv.IMWrite("result.png", img)
}
//}

Mat形式はn次元の密集した数値のシングルチャネルまたはマルチチャネル配列を表し、OpenCVでよく使われるデータ構造です。Mat形式のデータを@<code>{classifier.DetectMultiScale}に渡せば、顔部分を囲った範囲である image.Rectangle型(もうすでにお馴染み) のスライスが返ってきます。後は@<code>{gocv.Rectangle}関数で顔部分を囲み、画像として出力します。これを実行してみると@<img>{po3rin-facedetect}のような画像ができるはずです。

//image[po3rin-facedetect][gocvを使った顔認識][scale=0.5]{
//}



===[column] GoCVのパッケージにある import C について

Goはcgoを使ってCのコードを簡単に呼び出せる機能を提供しています。GoCVのソースコードを見るとOpenCVのCのソースコードをgocvで読み込んでいます。

//list[cgo][cgoを使ってCのコードを読み込み][go]{
package gocv

/*
#include <stdlib.h>
#include "imgproc.h"
*/
import "C"

func (c *CascadeClassifier) DetectMultiScale(img Mat) []image.Rectangle {
	ret := C.CascadeClassifier_DetectMultiScale(c.p, img.p)
	defer C.Rects_Close(ret)

	return toRectangles(ret)
}
//}

===[/column]


=== 輪郭抽出とマスクを使った画像合成

顔認識を行いましたが、せっかくなのでgocvのexampleにない物を実装しましょう。OpenCVを使った輪郭抽出からの画像合成を行います。今回は輪郭で切り抜きしたGopherくんを海で遊ばせましょう。@<img>{po3rin-drawflow}が今回のDrawMaskの概要になります。

//image[po3rin-drawflow][今回のDrawMaskの概要][scale=0.8]{
//}

ここでのポイントはGopehr君の輪郭を認識してマスクを作ることです。ここでもOpenCVが威力を発揮します。OpenCVを使っていく前に、まずはヘルパー関数を3つ作ります。まずは@<img>{po3rin-contours_flow}マスクを作るまでのフローを確認しましょう。マスクを作るまでにsrcに対してグレーケール化、2値化、輪郭抽出&塗りつぶしが行われます。この結果を使ってマスクを作ります。

//image[po3rin-contours_flow][Mask生成までのフロー][scale=1]{
//}

ここでOpenCVを使っていく前に今回使う関数を作っておきます。3つ目の関数は輪郭抽出した後の画像からmaskを作る関数です。

//list[ready-contours][画像合成のための準備][go]{
// Mat型からImage.Image型に変換する
func matToImage(fileExt gocv.FileExt, mat gocv.Mat) image.Image {
	srcb, _ := gocv.IMEncode(fileExt, mat)
	src, _, _ := image.Decode(bytes.NewReader(srcb))
	return src
}

// 標準入力から draw.Image に変換して返す
func getDst() draw.Image {
	s, _ := png.Decode(os.Stdin)
	dst, _ := s.(draw.Image)
	return dst
}

// 白色箇所をだけを使ったmaskを作る
func white2mask(src image.Image) image.Image {
	bounds := src.Bounds()
	dst := image.NewRGBA(bounds)
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			var col color.RGBA
			c := color.Gray16Model.Convert(src.At(x, y))
			gray, _ := c.(color.Gray16)
			if gray != color.Black {
				col = color.RGBA{0, 0, 0, 255}
			}
			dst.Set(x, y, col)
		}
	}
	return dst
}
//}

さて、いよいよ次は、OpenCVを使った輪郭抽出です。

===[column] GoCVのオススメの調べ方

GoCVのドキュメントはexampleが充実しているとはいえ、全ての関数やメソッドの解説を網羅している訳ではありません。その為、OpenCVを初めて使う場合は、PythonやC++の実装例からOpenCVの実装方法を調べることをお勧めします。幸運なことに、GoCVはOpenCVが提供する関数名や引数がほぼ一致しているので調べるのには苦労しないでしょう。

===[/column]

早速実装に入りましょう。

//list[contours][OpenCVを使った画像の輪郭抽出と合成][go]{
func main() {
	// 画像を読み込んでグレイスケール化
	cvtSrc := gocv.IMRead("./src/gopher.png", gocv.IMReadColor)
	gray := gocv.NewMatWithSize(460, 460, gocv.MatTypeCV64F)
	gocv.CvtColor(cvtSrc, &gray, gocv.ColorBGRToGray)

	// 二値化
	thresholdDst := gocv.NewMatWithSize(460, 460, gocv.MatTypeCV64F)
	gocv.Threshold(gray, &thresholdDst, 205, 255, gocv.ThresholdBinaryInv)

	// 輪郭抽出
	points := gocv.FindContours(thresholdDst,
		gocv.RetrievalExternal,
		gocv.ChainApproxSimple,
	)

	// 輪廓抽出結果からマスクの準備
	gocv.DrawContours(
		&thresholdDst, points, -1,
		color.RGBA{255, 255, 255, 255}, -1
	)
	maskSrc := matToImage(gocv.PNGFileExt, thresholdDst)
	mask := white2mask(maskSrc)

	src := matToImage(gocv.PNGFileExt, cvtSrc)
	r := src.Bounds()

	// 背景画像所得
	dst := getDst()

	// 合成
	draw.DrawMask(dst, r, src, image.Pt(0, 0), mask, image.Pt(0, 0), draw.Over)
	png.Encode(os.Stdout, dst)
}
//}

ここではgocvパッケージの中の多くの関数を使っています。一個ずつ見ていきましょう。

@<code>{gocv.CvtColor}関数では第三引数に変換したいMatTypeを指定することでカラーを変換できます。

//list[cvtColor][gocv.CvtColor][go]{
func CvtColor(src Mat, dst *Mat, code ColorConversionCode)
//}

@<code>{gocv.Threshold}関数では画像の2値化を行なっています。引数に渡す閾値(@<code>{thresh})によってどの画素値を境に2値化するか決定します。今回は画像に合わせて最適な閾値を決定しました。ThresholdTypeは閾値の種類です。@<code>{gocv.ThresholdBinaryInv}は閾値を超えた箇所は白に、他はmaxvalueに変換するthresholdTypeです。

//list[threshold][gocv.Threshold][go]{
func Threshold(
	src Mat, dst *Mat, thresh float32, maxvalue float32, typ ThresholdType
)
//}

@<code>{gocv.FindContours}関数で、輪郭抽出を行なっています。引数の@<code>{mode}で輪郭抽出するモードを選ぶことができます。今回使っているRetrExternalモードは輪郭のうち最も外側の輪郭のみを抽出するモードです。@<code>{method}では輪郭の近似手法を指定します。輪郭上の全点の情報を保持してしまうのは無駄なので、@<code>{gocv.ChainApproxSimple}は水平・垂直・斜めの線分を圧縮し、それらの端点のみを残します．これにより無駄なメモリの使用を抑えられます。

//list[findContours][gocv.FindContours][go]{
func FindContours(
	src Mat, mode RetrievalMode, method ContourApproximationMode
) [][]image.Point
//}

この輪郭抽出の結果を使って@<code>{gocv.DrawContours}関数で輪郭の中を白色で塗りつぶします。contoursは輪郭のデータです。第3引数contourIdxは描画したい輪郭のインデックス(第2引数で与えた輪郭のsliceから1つの輪郭だけを描画したいときに輪郭の指定に使います。全輪郭を描画する時はー1を指定します。他の引数は文字通り色(color)と線の太さ(thickness)を表します。

//list[drawContours][gocv.DrawContours][go]{
func DrawContours(
	img *Mat, contours [][]image.Point, contourIdx int,
	c color.RGBA, thickness int
)
//}

それぞれの関数を簡単に紹介しましたが、引数の意味はOpenCVのドキュメント@<fn>{opencvdoc}が詳しいのでこちらを参照ください。これで海で遊ぶGopherくんの画像を作成できました。当然、標準入力を変えれば、色んな背景でGopherくんを遊ばせてあげることができます。GoCVを使ったサンプルコードはドキュメント@<fn>{gocvdoc}で多く紹介されているので、こちらをぜひ試してみてください。
//footnote[opencvdoc][https://docs.opencv.org/3.3.0/index.html]
//footnote[gocvdoc][https://gocv.io/writing-code/more-examples/]

== おわりに

本章ではGoで画像を扱う際の基本的な知識から、実装方法まで解説しました。Goで画像を自由自在に扱えるイメージを持っていただき、今後の開発に活かしていただけると幸いです。
