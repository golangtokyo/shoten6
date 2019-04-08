= VimでGoを快適に書く

== はじめに
はじめまして、都内でゴリラ.vim@<fn>{gorilla.vim}を主催しているVim好きなゴリです。
普段はVimを使ってGoのコードを書くことが趣味でTUI@<fn>{tui}を作るのが好き、
最近はDocker@<fn>{docker}のTUI Clientツール@<fn>{docui}を作っています。

本章ではVimでGoを快適に書くための環境構築から設定やプラグインなどを紹介していきます。

//footnote[tui][Text User Interface]
//footnote[gorilla.vim][https://gorillavim.connpass.com/]
//footnote[docker][https://www.docker.com/]
//footnote[docui][https://github.com/skanehira/docui]

なお、本章は@<table>{env}をもとに解説していきます。

//table[env][環境]{
OS	Mac Mojave 10.14.4（18E226）
Vim	8.1.950
Go	go1.12 darwin/amd64
//}

また、本章の中に{HOME}が複数回出ますが、@<table>{home}の通りに読み替えてください。

//table[home][HOMEの定義]{
OS	パス
----------------------------
Mac	/Users/yourname
Linux	/home/yourname
Windows	C:\Users\yourname
//}

== Vimの基本

=== モードについて
Vimにはモードと言う概念があり、基本的にモードを切り替えながらコーディングします。
それぞれのモードでできる代表的な操作をまとめると@<table>{mode}のようになります。

//table[mode][モード]{
モード		操作
----------------------------
normal	カーソル移動
.	文字削除
.	文字コピー
.	文字貼り付け
.	アンドゥ
.	リドゥ
insert	文字入力
.	文字削除
.	文字貼り付け
visual	範囲選択
.	選択範囲を削除
.	選択範囲をコピー
.	範囲選択の解除
commandline	ファイルの保存
.	Vim終了
.	画面分割
.	置換
search	検索
//}

=== 基本操作
Vimを触ったことがない方は@<table>{operations}の基本操作を覚えておきましょう。

//table[operations][操作]{
モード	操作	キー	補足
----------------------------
normal	カーソル移動(左)	h
.	カーソル移動(右)	l
.	カーソル移動(上)	k
.	カーソル移動(下)	j
.	行コピー	yy, Y
.	削除	dd, D
.	貼り付け	p, P
.	アンドゥ	u
.	リドゥ		Ctrl+r
.	visual modeに移る	v	範囲選択
.	.	V	行単位で範囲選択
.	.	Ctrl+v	矩形範囲選択
.	insert modeに移る	i
.	search modeに移る	/
.	commandline modeに移る	:
insert	normal modeに戻る	esc, Ctrl+c
visual	選択した範囲をコピーする	y
.	選択した範囲を削除する	d
.	選択した範囲を削除して貼付け	p
.	normal modeに移る	esc, Ctrl+C
commandline	保存	w
.	Vimを終了する	q
.	保存してVimを終了	wq
.	保存せずVimを終了	q!
.	文字を置換する	%s/置換対象/置換文字/g
//}

== vim-go
Vimではプラグインを導入して機能を拡張することができます。
Goのコードを書くときに欠かせないプラグインとしてvim-go@<fn>{vim-go}があります。
定義ジャンプ、補完、linterなどの便利な機能があります。

//footnote[vim-go][https://github.com/fatih/vim-go]

=== 導入
Vimでは一般的にプラグインマネージャを使用してプラグインを管理します。
ここでは筆者が使用しているdein.vim@<fn>{dein.vim}というプラグインマネージャをもとに解説していきます。

なお、他のプラグインマネージャでも同様にプラグインを導入することが可能です。

//footnote[dein.vim][https://github.com/Shougo/dein.vim] 

==== vim-goの導入
vim-goを導入するため、{HOME}/.vim/dein.tomlという名前のファイルを作成して@<list>{add_vim-go}を先頭に追加してください。
なお、Gitが必要なので事前にインストールしておいてください。

//list[add_vim-go][dein.toml]{
# Go
[[plugins]]
repo = 'fatih/vim-go'
on_ft = 'go'
//}

dein.vimを導入するため、@<list>{install_dein.vim}を{HOME}/.vimrcの先頭に追加してください。

//list[install_dein.vim][vimrc]{
" install dir
let s:dein_dir = expand('~/.cache/dein')
let s:dein_repo_dir = s:dein_dir . '/repos/github.com/Shougo/dein.vim'

" dein installation check
if &runtimepath !~# '/dein.vim'
 if !isdirectory(s:dein_repo_dir)
 execute '!git clone https://github.com/Shougo/dein.vim' s:dein_repo_dir
 endif
 execute 'set runtimepath^=' . s:dein_repo_dir
endif

" begin settings
if dein#load_state(s:dein_dir)
 call dein#begin(s:dein_dir)

 " .toml file
 let s:rc_dir = expand('~/.vim')
 let s:toml = s:rc_dir . '/dein.toml'

 " read toml and cache
 call dein#load_toml(s:toml, {'lazy': 0})

 " end settings
 call dein#end()
 call dein#save_state()
endif

" plugin installation check
if dein#check_install()
 call dein#install()
endif

" enable syntax
syntax enable

" enable plugin with filetype
filetype plugin indent on
//}

追加後、Vimを立ち上げるとdein.vimとvim-goのインストールが開始され、しばらくすると終わります。
vim-goのインストール完了後、外部コマンドをインストールする必要があるため、
Vimのcommandline modeで@<code>{:GoInstallBinaries}を実行してください。

これでvim-goのインストールは完了です。

== vim-goコマンド
vim-goのコマンドやキーマッピング、設定を紹介していきます。

=== 定義ジャンプ
@<code>{Ctrl+]}もしくは@<code>{gd}でカーソル上の識別子の定義元へジャンプすることができます。
ジャンプ履歴は記録されるので@<code>{Ctrl+t}で前の場所に戻ることができます。

=== 整形
@<code>{:GoFmt}でソースコードを整形できます。
vim-goではファイル保存時に自動整形するようになっています。

=== インポート
@<code>{:GoImports}で必要なパッケージをインポート、不要なパッケージを削除することができます。
また、@<list>{set_go_fmt_command}の設定を{HOME}/.vimrcに追加することでファイル保存時に自動実行するようになります。

//list[set_go_fmt_command][vimrc]{
let g:go_fmt_command = "goimports"
//}

=== linter
@<list>{set_go_metalinter_autosave}の設定を{HOME}/.vimrcに追加することでファイル保存時に自動でlinterを実行することができます。

//list[set_go_metalinter_autosave][vimrc]{
let g:go_metalinter_autosave = 1
//}

デフォルトは@<code>{GoLint}、@<code>{GoVet}、@<code>{GoErrCheck}が有効になっていますが、
個別で指定したい場合は@<list>{set_go_metalinter_autosave_enabled}の設定を{HOME}/.vimrcを追加してlinterを指定してください。

//list[set_go_metalinter_autosave_enabled][vimrc]{
" vet, golint, errcheckを個別指定する
let g:go_metalinter_autosave_enabled = ['vet', 'golint']
//}

=== 関数(メソッド)間の移動
@<code>{]]}で次の、@<code>{[[}で前の関数(メソッド)へ移動することができます。
@<code>{N]]}、@<code>{N[[}のように数値を入力することで@<code>{]]}、@<code>{[[}を@<fn>{n}N回実行することもできます。

//footnote[n][Nは数値です]

=== テキストオブジェクト
関数内にカーソルを置き、@<code>{dif}で関数の中身、@<code>{daf}で関数全体を削除できます。
同様に@<code>{yif}で関数の中身、@<code>{yaf}で関数全体をコピーすることができます。
関数全体のコピー、削除はコメントも含まれます。

=== タグの追加
構造体のフィールドにカーソルを置き、@<code>{:GoAddTags}で@<list>{goaddtags_before}、@<list>{goaddtags}のようにタグを追加することができます。
タグは@<list>{goaddtags}の@<code>{``}で囲っている部分で、@<code>{json}はタグのキーです。

//list[goaddtags_before][実行前]{
type Gorilla struct {
	name string
	age  int
}
//}

//list[goaddtags][実行後]{
type Gorilla struct {
	name string `json:"name"`
	age  int    `json:"age"`
}
//}

また、@<code>{:GoAddTags yaml toml}のように引数を渡すことで@<list>{goaddtags_yaml_toml}のように複数のタグのキーを追加することもできます。

//list[goaddtags][実行前]{
type Gorilla struct {
	name string `json:"name"`
	age  int    `json:"age"`
}
//}


//list[goaddtags_yaml_toml][実行後]{
type Gorilla struct {
	name string `json:"name" yaml:"name" toml:"name"`
	age  int    `json:"age" yaml:"age" toml:"age"`
}
//}

=== タグの削除
@<list>{removetag_before}、@<list>{removetag_before}のように構造体のフォールドにカーソルを当てた状態で@<code>{:GoRemoveTag}で構造体のタグを削除することができます。

//list[removetag_before][実行前]{
type Gorilla struct {
	name string `json:"name" yaml:"name" toml:"name"`
	age  int    `json:"age" yaml:"age" toml:"age"`
}
//}

//list[removetag][実行後]{
type Gorilla struct {
	name string
	age  int
}
//}

また、@<code>{:GoRemoveTag toml}のように引数を渡すことで、@<list>{removetag_specific_before}、@<list>{removetag_specific}のように指定したタグのキーを削除することもできます。

//list[removetag_specific_before][実行前]{
type Gorilla struct {
	name string `json:"name" yaml:"name" toml:"name"`
	age  int    `json:"age" yaml:"age" toml:"age"`
}
//}

//list[removetag_specific][実行後]{
type Gorilla struct {
	name string `json:"name" yaml:"name"`
	age  int    `json:"age" yaml:"age"`
}
//}

=== GoDocを開く
@<code>{:GoDocBrowser}でカーソル上の定義のGoDocを開くことができます。
Vim上でGoDocを確認したい場合は@<code>{K}で確認できます。

=== インターフェイス実装一覧
インターフェイスにカーソルを置き@<code>{:GoImplements}で実装一覧を検索することができます。
検索結果を選択してEnterを押すことで実装にジャンプできます。

=== 構造体リテラルをゼロ値で初期化
@<code>{:GoFillStruct}で次のように構造体のをゼロ値で初期化することができます。
どんなフィールドがあったか忘れたときなどに使うと便利です。

//list[gofillstruct_before][実行前]{
type Gorilla struct {
	name string
	age int
}

func NewGorilla() Gorilla {
	return Gorilla{}
}
//}

//list[gofillstruct][実行後]{
type Gorilla struct {
	name string
	age int
}

func NewGorilla() Gorilla {
	return Gorilla{
		name: "",
		age:  0,
	}
}
//}

=== フィールド名付き構造体リテラル
@<code>{:GoKeyify}で@<list>{gokeyify_before}、@<list>{gokeyify}のようにフィールド名なしの構造体リテラルにフィールド名を追加することができます。
構造体の定義をリファクタリングするときに便利です。

//list[gokeyify_before][実行前]{
type Gorilla struct {
	name string
	age int
}

func NewGorilla() Gorilla {
	return Gorilla{"gorilla", 26}
}
//}

//list[gokeyify][実行後]{
type Gorilla struct {
	name string
	age int
}

func NewGorilla() Gorilla {
	return Gorilla{
		name: "gorilla",
		age:  26,
	}
}
//}

=== リネーム
@<code>{:GoRename}でカーソルを当てた変数や関数が使われている箇所をすべてリネームできます。

=== インターフェイスのメソッドスタブ生成
@<code>{:GoImpl}で@<list>{goimpl_before}、@<list>{goimpl}のようにカーソルを当てた型@<code>{T}に指定したインターフェイスのメソッドスタブを生成できます。
インターフェイスは、"インポートパス.インターフェイス名"の形で指定する必要があります。
ただし、Goの標準パッケージで提供されるインタフェースはインポートパスを省いても構いません。
この機能は、テスト用にモックを手早く作る場合に便利でしょう。

//list[goimpl_before][実行前]{
type T struct{}
//}

//list[goimpl][実行後]{
// io.ReadWriteCloserを指定した場合
type T struct{}

func (t *T) Read(p []byte) (n int, err error) {
	panic("not implemented")
}

func (t *T) Write(p []byte) (n int, err error) {
	panic("not implemented")
}

func (t *T) Close() error {
	panic("not implemented")
}

//}

=== コードシェア
@<code>{:GoPlay}でコードをThe Go Playground@<fn>{playground}にアップロードできます。
Vimで書いたコードをThe Go Playground上でシェアする場合に便利です。

//footnote[playground][https://play.golang.org/]

=== テスト実行
@<code>{:GoTest}でテストを実行することができます。
また、指定したテスト関数だけを実行したい場合はカーソルをテスト関数に当てた状態で@<code>{:GoTestFunc}で実行することができます。

=== カバレッジ
@<code>{:GoCoverage}でテストを実行し、カバレッジを確認することができます。
ブラウザでカバレッジを確認したい場合は@<code>{:GoCoverageBrowser}で確認できます。

=== スニペット
{HOME}/.vim/dein.tomlに@<list>{add_ultisnips}のプラグインを追加して、Vimを再起動してください。

//list[add_ultisnips][dein.toml]{
# snippets
[[plugins]]
repo = 'SirVer/ultisnips'
//}

vim-goが用意しているスニペット一覧@<fn>{snipps}を参照してください。

//footnote[snipps][https://github.com/fatih/vim-go/blob/master/gosnippets/UltiSnips/go.snippets,]

=== 補完
vim-goを導入することで@<code>{Ctrl+x + Ctrl+o}で補完できます。
2019/04/08でvim-goはLSP@<fn>{lsp}に対応したので、Go公式の@<fn>{gopls}goplsを使用して補完と定義ジャンプが可能になっています。
goplsを使用したい場合、@<list>{set_go_def_mode}の設定を{HOME}/.vimrcに追加してください。

//list[set_go_def_mode][vimrc]{
let g:go_def_mode = 'gopls'
//}

//footnote[lsp][Language Server Protocol(LSP)]
//footnote[gopls][https://github.com/golang/go/wiki/gopls]

== Vim編集時短術
vim-goを使用することで、VimでGoを快適にかけるようになりました。
Vimにはもっとたくさんの機能がありますが、ここでは便利機能をすこし紹介していきます。
なお、記述がなければnormal modeでの操作になります。

=== コメントアウト
@<code>{Ctrl+v}で矩形選択できるようになります。
矩形選択でコメントアウト範囲を選択したのち、@<code>{I//Esc}で選択した範囲をコメントアウトできます。

=== 構造体の中身をコピーと削除
構造体の中身をコピーしたい場合、カーソルを構造体内に置き、@<code>{yi{}でコピー、@<code>{di{}で削除できます。

=== 関数の引数をコピーと削除
関数の引数をコピーしたい場合、カーソルを()内に置き、 @<code>{yib}でコピー、@<code>{dib}で削除できます。

=== 前後の空白にジャンプする
@<code>{{}で上、@<code>{\}}で下の空白の部分にカーソルをジャンプさせることができます。

=== 改行してinsert mode
@<code>{o}で下に一行改行してinsert modeになります.
@<code>{O}で上に一行改行になります。

=== セッション
現在の作業(開いているファイル、画面分割、タブ)状態を保存するためのセッション機能があります。
一旦パソコンを再起動するときなど、何かしらの理由で作業を中断せざるを得ない場合に使うと便利です。
@<code>{:mksession xxx.vim}でセッションファイルを保存します。
保存したセッションファイルを@<code>{vim -S xxx.vim}で復元することができます。
またはVimを開いた状態であれば@<code>{:source xxx.vim}で復元することもできます。

=== 単語を修正
@<code>{ciw}で単語を削除してinsert modeに移ることができます。
次のように@<code>{ciw}実行後にWorldを入力することで単語を変更することができます。

//list[cw][実行前]{
// |がカーソルの位置
Hello |Gorilla
//}

//list[cw][実行後]{
// |がカーソルの位置
Hello World|
//}

=== 単語検索
単語にカーソルを当てた状態で@<code>{*}で検索できます。
@<code>{n}で次、@<code>{N}で前の検索結果にジャンプします。

=== 画面分割
@<code>{:vs}で水平分割、@<code>{:sp}で垂直分割できます。
また、@<code>{:vsp gorilla.go}のように引数を渡すことでファイルを分割した画面で開くことができます。

=== タブ
@<code>{:tabnew}でタブを新規作成することができます。
画面分割同様、@<code>{:tabnew gorilla.go}のように引数を渡すことで指定したファイルをタブで開くことができます。

また、@<code>{gt}もしくは@<code>{gT}でタブを切り替える事ができます。

== おまけ
=== オススメVimrc設定
オススメの設定をすこし紹介していきます。
@<list>{settings_vimrc}を{HOME}/.vimrcに追加することで少しVimが使いやすくなると思います。

//list[settings_vimrc][vimrc]{
" 文字コード
set encoding=utf-8
set fileencodings=utf-8,iso-2022-jp,euc-jp,sjis
set fileformats=unix,dos,mac

" シンタックスを有効にする
syntax enable

" ファイル形式別プラグインとインデントを有効にする
" プラグインを使用するには設定する必要がある
filetype plugin indent on

" バックスペースとCtrl+hで削除を有効にする
set backspace=2

" 改行時自動インデント
set smartindent

" 行番号を表示
set number

" カーソルから相対的な行数を表示する
set relativenumber

" インクリメントサーチを有効にする
set incsearch

" 検索時大文字小文字を区別しない
set ignorecase

" 検索時に大文字を入力した場合ignorecaseが無効になる
set smartcase

" 検索結果をハイライトする
set hlsearch

" カーソルラインを表示する
set cursorline

" wildmenuを有効にする
set wildmenu
set wildmode=full

" undoの保存先
" 事前にディレクトリを作成しておく必要がある
if has('persistent_undo')
  set undodir=~/.vim/undo
  set undofile
endif

" カーソルラインの位置を保存する
if has("autocmd")
    autocmd BufReadPost *
    \ if line("'\"") > 0 && line ("'\"") <= line("$") |
    \   exe "normal! g'\"" |
    \ endif
endif

" grepした結果をquickfixに表示する
augroup grepwindow
    autocmd!
    au QuickFixCmdPost *grep* cwindow
augroup END

" 矩形選択時に文字の無いところまで選択範囲を広げる
set virtualedit=block

" 行先頭と行末
map H ^
map L $

" visual時に選択行を移動
vnoremap <C-j> :m '>+1<CR>gv
vnoremap <C-k> :m '<-2<CR>gv

" 上下の空白に移動
nnoremap <C-j> }
nnoremap <C-k> {

" 検索でvery magicを使用する
" `("`と言った記号をエスケープせずに検索できる
nnoremap /  /\v

" ハイライトを削除する
nnoremap <Esc><Esc> :nohlsearch<CR>
//}

=== オススメプラグイン
オススメのプラグインを紹介します。
@<list>{plugins}の設定を{HOME}/.vim/dein.tomlに追加して、Vimを再起動してください。

//list[plugins][vimrc]{
[[plugins]]
repo = 'fatih/vim-go'
on_ft = 'go'

# ファイル検索
[[plugins]]
repo = 'junegunn/fzf'
build = './install --all'

[[plugins]]
repo = 'junegunn/fzf.vim'

# "や'などを自動で閉じてくれる
[[plugins]]
repo = 'cohama/lexima.vim'

# 分割した画面のサイズ調整
[[plugins]]
repo = 'simeji/winresizer'

# ステータスバーカスタマイズ
[[plugins]]
repo = 'itchyny/lightline.vim'
hook_add = '''
  let g:lightline = {
    \ 'colorscheme': 'wombat',
    \ 'active': {
        \ 'left': [ ['mode', 'paste'], ['readonly', 'filepath', 'modified'] ]
        \ },
    \ 'component_function':{
        \ 'filepath': 'FilePath'
        \ }
    \ }

  function! FilePath()
    if winwidth(0) > 90
      return expand("%:s")
    else
      return expand("%:t")
    endif
  endfunction

  " # show statusbar
  set laststatus=2
  " # hide --INSERT--
  set noshowmode
'''

# セッション管理プラグイン、fzfと合わせて使うと便利
[[plugins]]
repo = 'skanehira/vsession'

# 置換結果をリアルタイムで表示
[[plugins]]
repo = 'markonm/traces.vim'
//}

== おわりに
本章ではVimでGoを快適に書くためについて紹介しました。
まだまだVimにはたくさん便利な機能がありますので、気になる方はぜひ調べてみてください。

Vimは慣れてしまえばとても快適にコーディングすることができますので、
ぜひ慣れてコーディングスピードを上げていきましょう。
