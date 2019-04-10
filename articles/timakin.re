= Goの画像合成デザインパターン

== はじめに

株式会社グノシー バックエンドエンジニアの@__timakin__@<fn>{timakin}です。
本章では、Goの画像処理パッケージを活用した画像合成処理のデザインパターンを、現場での応用例を踏まえながら紹介します。

//footnote[timakin][@<href>{https://twitter.com/__timakin__}]

== imageパッケージの概要

Goの標準パッケージの一つであるimage@<fn>{image}パッケージは、その名の通り2D画像を扱うためのパッケージです。
jpeg、gif、pngなど複数の画像形式に対応しています。
このパッケージの機能は、大きく分けると3つです。

 * 画像の生成
 * 画像情報の取得
 * 画像の加工

po3rinさんの章で画像解析の手法の概要が整理されているので、本章の事例で用いる処理とパッケージにフォーカスして再整理します。

//footnote[image][@<href>{https://golang.org/pkg/image/}]

=== 画像の生成

本章で画像を新しく生成する際、@<code>{image.NewRGBA}を使います。
@<code>{image.NewRGBA}は@<code>{RGBA}の構造体を返します。
この構造体は、画像の情報をメモリ上に格納する用途で使います。具体的には、座礁ごとのピクセル情報が格納されます。

それでは、画像を生成してみましょう。@<list>{rectdraw}では、200x200のサイズの画像のデータを初期化した後に、緑色で塗りつぶしています。
さらに、この例では@<code>{draw}パッケージを利用しています。成果物の画像の枠となる構造体と、付加する情報を渡すことで、画像が生成されます。

//list[rectdraw][image.NewRGBAの使用例][go]{
func main() {
    rect := image.NewRGBA(image.Rect(0, 0, 200, 200))
    green := color.RGBA{0, 100, 0, 255}
    draw.Draw(rect, rect.Bounds(), &image.Uniform{green}, image.ZP, draw.Src)
}
//}

=== 画像情報の取得

@<code>{image}パッケージでは、すでにある画像の情報を取得することも可能です。
取得できるのは、ピクセル単位での色情報や画像の縦横サイズなどです。
特に大きさの取得は本章での画像処理に大きく関わってきます。

=== 画像の加工

画像の情報を読み取るだけでなく、加工を施すこともできます。
ピクセルを個々に処理することで、色情報を変更したり、画像の一部に画像を重ねたり、
更には@<code>{draw.DrawMask}というメソッドを使用すれば枠として用意した画像を元に、
別の画像にマスクすることが可能です。

ピクセル単位での細かい加工は一見すると難易度が高く感じられますが、
インターフェースが綺麗に整備されているおかげで、多少ドキュメントを読み進めれば誰でも画像が加工できるようなレベルに落ち着いています。

さらに、他の言語と比べて特徴的なのは、依存するミドルウェアの導入が不要なことです。
例えば@<code>{Ruby}を使ったことのある方ならご理解いただけると思いますが、@<code>{RMagick}や@<code>{minimagick}などのソフトウェアをホストとなる環境にインストールしておかなければ、
画像処理を行えないケースがあります。場合によっては環境ごとにそれらツール導入に手間取ることも少なくはありません。
Goの@<code>{image}パッケージやそのラッパーなら、即座に画像を読み書きすることが可能な点は、開発を始めやすい点で大きなメリットです。

ラッパーと書きましたが、標準の@<code>{image}パッケージだけでなく、使い勝手の良い画像処理ラッパーもGoにはいくつか存在します。
その中でもシンプルなインターフェースと必要十分な機能性を備えたパッケージとして、imaging@<fn>{imaging}パッケージをご紹介します。

//footnote[imaging][@<href>{https://github.com/disintegration/imaging}]

このパッケージでは、標準パッケージがサポートする画像形式に加えて、標準化までは至っていない@<code>{golang.org/x/image}配下の機能も含めてラップし、bmpやtiffなどの形式も同じインターフェースを通じて読み込み、書き出しすることが可能です。
さらに良い点として、画像加工周りが充実していることでしょう。具体的には次のような機能が備わっていて、インスタグラムで行うようなフィルターをカバーできるような充実度となっています。

 * リサイズ（単純なリサイズに加え、特定の値にフィットするようなリサイズも可能）
 * シャープネス調整
 * 明るさ調整
 * 色調調整
 * ぼかし
 * 切り抜き

=== テキスト描画

@<code>{image}パッケージには備わっていないものの、@<code>{golang.org/x/image/font}に準公式機能として実装されているものの中に、画像へのテキストの描画があります。
@<list>{addLabel}では、画像の特定の座標にテキストを表示するためのメソッドを定義しました。
@<code>{golang.org/x/image/font/basicfont}というパッケージを利用してフォントを指定していますが、
これ以外にも@<code>{opentype}パッケージや@<code>{truetype}パッケージを利用すれば、自由にフォントをカスタマイズしながら
画像内にテキストを描画することが可能です。

//list[addLabel][テキストを画像に描画するメソッド][go]{
import (
    "golang.org/x/image/font"
    "golang.org/x/image/font/basicfont"
    "golang.org/x/image/math/fixed"
    "image"
    "image/color"
)

func addLabel(img *image.RGBA, x, y int, label string) {
    col := color.RGBA{200, 100, 0, 255}
    point := fixed.Point26_6{fixed.Int26_6(x * 64), fixed.Int26_6(y * 64)}

    d := &font.Drawer{
        Dst:  img,
        Src:  image.NewUniform(col),
        Face: basicfont.Face7x13,
        Dot:  point,
    }
    d.DrawString(label)
}
//}

== 本章での応用例

さて、@<code>{image}パッケージの概要を再整理しましたが、次はそれらをどのように本章で活用するか、具体的な事例とその利用目的を説明します。

=== 画像生成の利用目的

サーバーサイドで画像を動的に生成したいケースの中でも、カジュアルな事例はOGP画像の生成などかと思います。
TwitterやFacebookのSNSでシェアされるときに、自社サービスのコンテンツの色が出やすいフォーマットの画像を用意しておいて、
ページ内のテキストを一部重ねて動的に画像を作ることで、流入数を増やす目的の施策で、画像生成を行ったりするかと思います。

本章ではもう一歩踏み込んで、弊社で開発されたオトクル@<fn>{オトクル}というクーポンが掲載されるアプリで活用した、画像生成の方法についてご紹介します。
クーポンが掲載、閲覧できるアプリであれば、各クライアント店舗様毎のクーポン画像を大量に用意する必要があります。
今回題材としてご紹介する実装を行うまで、クーポン毎にデザイナーのチームが手作業で画像を作成し、その作業で1日を終えてしまうこともありました。
プロダクトの初期開発フェーズでは、環境やツールを用意する時間が取れないこともあるとは思いますが、人的資源が1日分消費されることが習慣化されるのは看過できません。

当課題を解決するために、テキストや画像を用意してそのパスなどを引数として渡せば、画像とテキストを特定フォーマットで合成できる仕組みを作りました。
本章でご紹介するのは、その大まかな実装の流れです。

//footnote[オトクル][@<href>{https://otokul.jp/}]

== 実装方法

「テキストや画像を用意してそのパスなどを引数として渡せば、画像とテキストを特定フォーマットで合成できる仕組み」としましたが、この実装に必要な要素はどのようなものがあるでしょうか。
本章では大きく分けて3つあると想定します。

 * 素材の読み込み
 * 画像及びテキストの合成
 * 画像の書き出し

また、この機能群を実装するために、いくつかインターフェースを用意します。
本章での用途に限ったインターフェースの設計なので、あくまで本章のコードを読むにあたってのみご参照ください。

 * Processor（画像合成コマンドの全体を実行）
 * Framer（画像とテキストを配置する枠組みの作成）
 * Drawer（画像の描画）
 * Labeler（テキストの描画）
 * Encoder（合成結果の書き出し）

これらのインターフェースを実装した構造体を、ファクトリとなる構造体を通じて初期化します。
なお、テキストの処理は画像との相対的な位置を特定しないとできないものも多いため、@<code>{Drawer}が@<code>{Labeler}を保持することにしました。
処理のエントリーポイントとなる@<code>{Proccessor}を、@<list>{proc}のように実装します。
@<code>{Run}という、各インターフェースの実装を呼び出す抽象的なメソッドを定義することで、クーポン画像以外の生成にも活用できるようになっています。

//list[proc][Proccessorの初期化と実行][go]{
type Processor interface {
	Run() error
}

type ProcessorType = string

const (
	CouponProcessorType = "coupon"
)

type ProcessorFields struct {
	Framer  Framer
	Drawer  Drawer
	Encoder Encoder
}

type CouponProcessor struct {
	ProcessorFields
}

func (cp *CouponProcessor) Run() error {
	frame := cp.Framer.Init()
	result, err := cp.Drawer.Draw(frame)
	if err != nil {
		return err
	}
	return cp.Encoder.Encode(result)
}

type ProcessorFactory struct{}

func (pf *ProcessorFactory) New(pt ProcessorType) (Processor, error) {
	switch pt {
	case CouponProcessorType:
		return newCouponProcessor()
	default:
		return nil, nil
	}
}
//}

=== 素材の読み込み

@<code>{Processor}のファクトリメソッドの先には、具体的な@<code>{Processor}実装があるはずです。
本章の例では、@<list>{newCouponProcessor}のような実装を行いました。成果物の書き出しは都合上jpegで行っています。

@<code>{imaging}パッケージを使って多様な画像形式に対応できるようにすると良いでしょう。
本章の事例でも、画像ファイルに関してクライアント様に規約を提示しつつ、形式の差異を一定程度吸収できるように@<code>{imaging}パッケージを利用しました。
また、画像合成は素材が多くなる可能性があります。そこで、固定で利用する素材はサイズ的に問題がなければstatik@<fn>{statik}を使ってあらかじめバイナリに含めても良いと思います。
そのため、当実装では細かいパーツだけ@<code>{statik}を使って読み出すようにしました。

//footnote[statik][@<href>{https://github.com/rakyll/statik}]

//list[newCouponProcessor][クーポン画像用Proccessor初期化処理][go]{
func newCouponProcessor() (Processor, error) {
	var (
		logo          = flag.String("logo", "xxx", "/path/to/logo")
		itemName      = flag.String("name", "xxx", "item name")
		itemImagePath = flag.String("item", "xxx", "/path/to/item_image")
		discountLabel = flag.String("discount", "xxx", "discount")
	)
	flag.Parse()

	statikFS, err := fs.New()
	if err != nil {
		return nil, err
	}

	itemFile, err := imaging.Open(*itemImagePath)
	if err != nil {
		return nil, err
	}

	logoFile, err := imaging.Open(*logo)
	if err != nil {
		return nil, err
	}

	priceBgFile, err := statikFS.Open(couponPriceBgPath)
	if err != nil {
		return nil, err
	}
	defer priceBgFile.Close()

	pbg, err := imaging.Decode(priceBgFile)
	if err != nil {
		return nil, err
	}

	logoMaskFile, err := statikFS.Open(couponLogoMaskPath)
	if err != nil {
		return nil, err
	}
	defer logoMaskFile.Close()

	lmimg, err := imaging.Decode(logoMaskFile)
	if err != nil {
		return nil, err
	}

	dsFile, err := statikFS.Open(couponDiscountSlashPath)
	if err != nil {
		return nil, err
	}
	defer dsFile.Close()

	dsimg, err := imaging.Decode(dsFile)
	if err != nil {
		return nil, err
	}

	p := new(CouponProcessor)
	p.Framer = &WhiteFramer{
		FrameSize: DefaultWhiteFrameSize,
	}
	p.Drawer = &CouponDrawer{
		Logo:          logoFile,
		LogoMask:      lmimg,
		Item:          itemFile,
		DiscountBg:    pbg,
		ItemLabel:     *itemName,
		DiscountLabel: *discountLabel,
		DiscountSlash: dsimg,
		Labeler:       new(CouponLabeler),
	}
	p.Encoder = new(JPEGEncoder)
	return p, nil
}
//}

=== 画像およびテキストの合成

画像およびテキストの合成に使われる@<code>{Framer}、@<code>{Drawer}、@<code>{Labeler}のインターフェースは、できる限りシンプルに拡張できるように、@<list>{dlinterface}のように最小限のメソッドに抑えます。
枠を与えられたら、それを描画先として画像とテキストを適切な座標に描画していく、という機能が実現できれば良いでしょう。

//list[dlinterface][クーポン画像用Proccessor初期化処理][go]{
type Framer interface {
	Init() *image.RGBA
}

type Drawer interface {
	Draw(*image.RGBA) (*image.RGBA, error)
}

type Labeler interface {
	Attach(*image.RGBA, string, *LabelAttachOption) error
}
//}

@<code>{Framer}は画像とテキストを配置する枠組みの作成の責務を担うとし、例えば単純に白い背景を用意する場合は、
サイズに合わせて、@<list>{whiteframer}にある@<code>{fillRect}メソッドのように、単色白背景で@<code>{image.RGBA}の構造体を用意してあげます。
単色で描画する場合は、@<code>{image.Rect}の隅から隅までに色を設定します。

//list[whiteframer][WhiteFramerの定義][go]{
type WhiteFramer struct {
	FrameSize *FrameSize
	BgColor   color.Color
}

type FrameSize struct {
	X, Y int
}

func (f *WhiteFramer) Init() *image.RGBA {
	frame := image.NewRGBA(image.Rect(0, 0, f.FrameSize.X, f.FrameSize.Y))
	fillRect(frame, color.White)
	return frame
}

// 画像を単色に染める
func fillRect(img *image.RGBA, col color.Color) {
	// 矩形を取得
	rect := img.Rect

	// 全部埋める
	for h := rect.Min.Y; h < rect.Max.Y; h++ {
		for v := rect.Min.X; v < rect.Max.X; v++ {
			img.Set(v, h, col)
		}
	}
}
//}

@<code>{Drawer}と@<code>{Labeler}に関しては実装が複雑なこともあり、一部だけご紹介します。
まず、@<code>{Drawer}が画像を描画する際は、@<list>{drawItem}のように@<code>{draw.Draw}メソッドを使います。
もし仮に異なるサイズの画像が来た時でもリサイズできるように、事前にリサイズ処理を実行しています。

//list[drawItem][商品画像の描画][go]{
func (cd *CouponDrawer) drawItem(frame *image.RGBA) {
	resizeRatio := float64(CouponItemMaxWidth) / float64(cd.Item.Bounds().Dx())
	resizedITem := imaging.Resize(
		cd.Item, 
		int(float64(cd.Item.Bounds().Dx())*resizeRatio), 
		int(float64(cd.Item.Bounds().Dy())*resizeRatio), 
		imaging.Lanczos)
	draw.Draw(frame, resizedITem.Bounds(), resizedITem, image.ZP, draw.Over)
}
//}

テキストを描画する際は、@<list>{loadFont}のようにまずフォントのデータを読み込まなければなりません。
@<code>{font.Drawer}という構造体を初期化していますが、この構造体が持つ@<code>{DrawString}メソッドが@<list>{drawLabel}のようなテキストの描画の役割を担います。
@<code>{fixed.Int26_6}という型が登場していますが、テキストを描画する座標を@<code>{font.Drawer}に渡す際は、この@<code>{fixed.Int26_6}型に変換する必要があるので、ご注意ください。

//list[loadFont][フォントの読み込み][go]{
fnt, err := loadFont(opt.FontFamily)
if err != nil {
    return err
}

ft, err := truetype.Parse(fnt)
if err != nil {
    return err
}

fopt := truetype.Options{Size: opt.FontSize}
face := truetype.NewFace(ft, &fopt)

d := &font.Drawer{
    Dst:  img,
    Src:  image.NewUniform(opt.Color),
    Face: face,
}
//}

//list[drawLabel][テキストの描画][go]{
labels := strings.Split(label, LineBreaker)
point := fixed.Point26_6{
    X: fixed.Int26_6(opt.LabelCoord.X*64 + opt.LabelOffset.X), 
    Y: fixed.Int26_6(opt.LabelCoord.Y*64 + opt.LabelOffset.Y),
}
labels = strings.Split(label, PriceShiftSeparater)

// 中央寄せで描画する場合
if opt.Centered {
    point = fixed.Point26_6{
        X: (fixed.I(img.Bounds().Dx()) - d.MeasureString(label)) / 2,
        Y: fixed.I(opt.LabelCoord.Y + opt.LabelOffset.Y),
    }
}
d.Dot = point
d.DrawString(label)
//}

@<code>{Labeler}が実装されたら、@<list>{attachLabel}で描画したい位置、フォントの設定などのオプションを渡しながら描画の処理を実際に呼び出します。

//list[attachLabel][テキストの描画処理の呼び出し][go]{
// dはDrawerインターフェースの実装
err = d.Labeler.Attach(frame, d.DiscountLabel, &LabelAttachOption{
    LabelCoord: &coord{
        X: 0,
        Y: d.getDiscountBgOffset(frame).Y + (d.DiscountBg.Bounds().Dy() / 2),
    },
    LabelOffset: &coord{
        X: 0,
        Y: int(math.Ceil(CouponDiscountFontSize * CouponDiscountDpi / 72)),
    },
    FontFamily: GenShinGothicFontPath,
    FontSize:   CouponDiscountFontSize,
    Color:      CouponDiscountFontColor,
    Centered:   true,
})
//}

=== 画像の書き出し

最後に、@<code>{Encoder}です。@<list>{jpegencoder}では、jpegの画像を書き出す@<code>{Encoder}を実装しました。
なぜわざわざ@<code>{image}パッケージの@<code>{Encode}処理をラップしているかというと、画像の書き出し品質が決まっていたり、
他にもオプションを渡す必要が出て来たときに便利だからです。

//list[jpegencoder][Encoderの実装][go]{
type Encoder interface {
	Encode(*image.RGBA) error
}

type JPEGEncoder struct{}

func (je *JPEGEncoder) Encode(frame *image.RGBA) error {
	out, err := os.Create(defaultOutputPath)
	if err != nil {
		return err
	}
	return jpeg.Encode(out, frame, &jpeg.Options{
		Quality: 100,
	})
}
//}

素材の読み込みから画像の書き出しまで、一通り実装できたら、@<code>{Processor}をAPIサーバーやCLIとして呼び出します。
成果物は外部のストレージやローカルの特定のパスなど任意の位置に書き出すと良いでしょう。

== おわりに

本章では、実際に現場で使っている画像生成処理を題材に、
@<code>{image}パッケージやそれに関連する@<code>{golang.org/x/image}を用いて画像とテキストを合成する実装について見てきました。

fontの読み込みや画像処理を並列で実行する場合の効率的なワーカー実装など、まだまだ課題が多い分野ではありますが、
言語標準のパッケージでここまでシンプルに画像処理を実装できるインターフェースを持つものは、なかなかありません。
ぜひGoの@<code>{image}パッケージを実際のサービス開発に活用してみてください。
