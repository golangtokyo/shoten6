= vimでGoを快適に書く

== はじめに
始めまして、ゴリラです。

普段はvimを使ってGoを書くことが趣味で、@<br>{}
主にTUIツールを作ることが多いです。@<br>{}

本章ではvimでGoを快適に書くための環境構築から、vimの設定やTips、プラグインなどを紹介していきます。@<br>{}

なお、@<br>{}
本章は以下の環境をもとに解説していきます。

@<b>{OS}@<br>{}
Mac Mojave

@<b>{vim}@<br>{}
8.1.950

== vimの基本

=== モードについて

vimにはモードと言う概念があります。@<br>{}
基本モードを切り替えながらコーディングします。@<br>{}
それぞれのモードでできることは大まかに以下になります。

 * @<b>{normal mode}
 ** カーソル移動
 ** 文字削除
 ** 文字コピー
 ** 文字貼り付け
 ** undo・redo
 * @<b>{insert mode}
 ** 文字の入力
 ** 文字貼り付け
 * @<b>{visual mode}
 ** 範囲選択
 ** 選択範囲を削除
 ** 選択範囲をコピー
 * @<b>{commandline mode}
 ** ファイルの保存
 ** 画面分割
 ** 置換
 * @<b>{search mode}
 ** 検索

=== 基本操作
vimを触ったことがない方は、こちらの基本操作を覚えておきましょう。@<br>{}
基本操作をわかっている方は飛ばして頂いて問題ありません。@<br>{}
下記に記載がなければ、基本normal modeでの操作になります。

==== カーソル移動
カーソル移動は左（h）,右（l）,下（j）,上（k）になります。

==== コピー
`yy/Y`で行をコピーします。

==== 切り取り
`dd/D`で行を切り取ることができます。@<br>{}
切り取った行を貼り付けることができます。

==== 貼り付け
`p/P`でコピーした内容を貼り付けることができます。

==== undo・redo
`u` でundo、 `Ctrl + r` でredoになります。

==== 入力
`i`を入力するとinsert modeに切替わり、入力が可能になります。@<br>{}
insert modeで`esc`で抜けることができます。

==== 選択
`v`もしくは`V`でvisual modeになります。@<br>{}
このモードでは範囲選択ができ、選択した範囲に対して`y/Y/d/D/p/P`と行ったオペレーションが可能です。

==== 検索
`/`で検索モードになり、入力した文字を検索できます。@<br>{}
デフォルトでは検索結果をハイライトしないため、オプションで有効にする必要があります。@<br>{}
オプションについてはオススメvimrc設定で紹介します。

==== ファイル保存
`:`を入力すると、commandline modeに切り替わります。@<br>{}
`:`を入力したあとにつけてwを入力すると現在開いたファイルの変更がファイルに書き込まれ保存されます。

==== 置換
`:%s/置換対象/置換文字/g`で置換できます。@<br>{}
`/`も置換対象の場合は`\/`でエスケープしなければのはご注意ください。

==== 終了
`:q`を入力するとvimを終了できます。@<br>{}
変更した内容を保存せずに、強制終了したい場合は`q!`でできます。

== vim-go
vimではプラグインを導入して機能を拡張することができます。@<br>{}
Goを書く時に、欠かせないプラグインとしてvim-goというのがあります。@<br>{}
定義ジャンプ、補完、linterなどの機能が詰まった素晴らしいプラグインです。@<br>{}
=== 導入
vimではプラグインマネージャを使用してプラグインを管理することが一般的です。@<br>{}
ここでは、筆者が使用しているdein.vimというプラグインマネージャをもとに解説していきます。@<br>{}
なお、他のプラグインマネージャも同様に導入することが可能です。@<br>{}

==== vim-goの導入
dein.vimの導入については、$HOME/.vimrcに以下の部分を先頭に追記してください。@<br>{}
なお、導入はgitが必要なので事前にインストールしておいてください。

===== $HOME/.vim/dein.tomlにファイルを作成して以下を先頭に追加してください。
//emlist[go][toml]{
# Go
[[plugins]]
repo = 'fatih/vim-go'
on_ft = 'go'
//}

===== dein.vimを導入するため、以下を$HOME/.vimrcの先頭に追加してください。
//emlist[dein][vimscript]{
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

上記を追加後、vimを立ち上げるとdein.vimとvim-goのインストールが開始され、しばらくすると終わります。@<br>{}
vim-goのインストール完了後、外部コマンドをインストールする必要があるため、@<br>{}
以下のコマンドをvimのcommand line modeで@<code>{:GoInstallBinaries}実行してください。

これでvim-goのインストールは完了です。

== vim-goコマンド
vim-goのオススメコマンドやキーバインド、設定を紹介していきます。@<br>{}

=== 定義ジャンプ
@<code>{Ctrl+]}もしくは@<code>{gd}でカーソル上にある定義へジャンプすることができます。@<br>{}
@<code>{Ctrl+]}はvim-go側が用意したキーマッピングです。@<br>{}
プラグイン内部でGoDefコマンドを実行しているだけです。@<br>{}
なので@<code>{:GoDef}を実行しても同じ事ができます。@<br>{}
ジャンプ履歴は記録されるので、前の場所に戻りたい時は@<code>{Ctrl+t}で戻ることができます。

=== フォーマット
@<code>{GoFmt}を実行すると、ソースコードが整形されます。@<br>{}
vim-goではファイル保存時に自動的にフォーマットしてくれるので、忘れても大丈夫です。

=== インポート
@<code>{:GoImports}で構文解析して、必要なパッケージをimportしてくれたり、不要なパッケージを削除してくれます。@<br>{}
また、以下の設定を$HOME/.vimrcに追加することでファイル保存時に自動実行するようになります。

//emlist[vimrc][vimscript]{
let g:go_fmt_command = "goimports"
//}

=== linter
以下の設定を$HOME/.vimrcに追加することで、ファイル保存時に自動でlinterをかけてくれます。

//emlist[vimrc][vimscript]{
let g:go_metalinter_autosave = 1
//}

デフォルトはGoLint,GoVet,GoErrCheckですが、
指定したい場合は以下の設定を$HOME/.vimrcを追加してください。

//emlist[vimrc][vimscript]{
let g:go_metalinter_autosave_enabled = ['vet']
//}

=== 関数移動
@<code>{]]}で次の関数,@<code>{[[}で前の関数へ移動することができます。@<br>{}

=== テキストオブジェクト
関数内にカーソルを移動し、@<code>{dif}で関数の中身、@<code>{daf}で関数全体を削除できます。@<br>{}
@<code>{yif}で関数の中身、@<code>{yaf}で関数全体をコピーできます。@<br>{}

=== タグの追加
構造体のフィールドにカーソルを当てた状態で@<code>{:GoAddTags}でタグを追加することができます。@<br>{}
引数がなければ、デフォルトはjsonタグが追加されますが、@<br>{}
@<code>{:GoAddTags yaml toml}の様に引数を渡すことで複数のタグを追加することができます。@<br>{}

=== タグの削除
構造体のフォールドにカーソルを当てた状態で@<code>{:GoRemoveTag}で構造体のタグを削除することができます。@<br>{}
引数を渡すことで、指定したタグを削除することもできます。

=== godocを開く
@<code>{:GoDocBrowser}でカーソル上の定義のgodocを開くことができます。@<br>{}
vim上でgodocを確認したい場合は@<code>{K}で確認することもできます。@<br>{}
筆者は後者のが好みです。

=== インターフェイス実装一覧
@<code>{:GoImplements}でカーソル上にあるインターフェイスを実装した構造体を探すことができます。@<br>{}
見つかった構造体はLocation Listに表示されるので、そこから選択してEnterを押すことでジャンプできます。

=== 構造体リテラルをゼロ値で初期化
@<code>{:FillStruct}で構造体のをゼロ値で初期化( T{} ) --> ( T{A: 1, B: 2, C: 3})することができます。@<br>{}
どんなキーがあったか忘れたときなどに使うと便利です。

=== キー付き構造体リテラル
@<code>{:GoKeyify}でキーなしの構造体リテラル( T{1, 2, 3} )をキー付き構造体リテラル( T{A: 1, B: 2, C: 3} )にすることができます@<br>{}
リファクタリングするときに便利です。

=== リネーム
@<code>{:GoRename}でカーソルにある変数や関数が使われている箇所をすべてリネームできます。@<br>{}

=== インターフェイスのメソッドスタブ生成
@<code>{:GoImpl}でカーソルを当てた構造体に、指定したインターフェイスの関数を自動生成できます。@<br>{}
インターフェイスを指定するとき、$GOPATH/src以降のパス.インターフェイス名で指定する必要があります。@<br>{}
ただし、Goの標準パッケージの場合はインターフェイス名のみで問題ないです。@<br>{}
大量にメソッドが定義されている場合、コマンド一つでスタブをつくれるのは魅力的ですね。

=== コードシェア
@<code>{:GoPlay}でソースをGoplaygroundにアップロードできます。@<br>{}
ローカルで書いたサンプルをシェアする時に便利です。

=== テスト実行
@<code>{:GoTest}でテストを実行することができます。@<br>{}
また、指定したテスト関数だけを実行したい場合はカーソルを関数に当てた状態で、@<code>{:GoTestFunc}で実行することで確認できます。

=== カバレッジ
@<code>{:GoCoverage}でテストを実行し、カバレッジを確認することができます。@<br>{}
ブラウザでカバレッジを確認したい場合は@<code>{:GoCoverageBrowser}を実行することで確認できます。

=== スニペット
$HOME/.vim/dein.tomlに以下のプラグインを追加して、vimを再起動してください。

//emlist[ultisnips][toml]{
# snippets
[[plugins]]
repo = 'SirVer/ultisnips'
//}

vim-goが用意しているスニペット一覧はこちらを参照してください。@<br>{}
https://github.com/fatih/vim-go/blob/master/gosnippets/UltiSnips/go.snippets

=== 補完
vim-goを導入すれば、Ctrl+x + Ctrl+oで補完できます。@<br>{}
執筆時点でvim-goはLSPに対応したので、Go公式のgoplsを使用して補完と定義ジャンプが可能になっています。@<br>{}
goplsを使用したい場合、以下の設定を$HOME/.vimrcに追加してください。

//emlist[vimrc][vimscript]{
let g:go_def_mode = 'gopls'
//}

== vim編集時短術
vim-goを使用することで、vimでGoを快適にかける様になりました。@<br>{}
vimにはもっとたくさんの機能がありますが、ここでは便利機能をすこし紹介していきます。@<br>{}
なお、記述がなければnormal modeでの操作になります。

=== コメントアウト
Ctrl + v で矩形選択できるようになります。@<br>{}
矩形選択でコメントアウト範囲を選択したのち、`I//Esc`を入力することで選択した範囲をコメントアウトできます。

=== 構造体の中身をコピー or 削除
構造体の中身をコピーしたい場合、カーソルを構造体内に置き、`yi{` でコピー `di{` で削除できます。

=== 関数の引数をコピー or　削除
関数の引数をコピーしたい場合、カーソルを()内に置き、 `yib` でコピー、 `dib` で削除できます。

=== 前後の空白にジャンプする
`{` で上、 `}` で下の空白の部分にカーソルをジャンプさせることができます。

=== 改行してinsert mode
`o` で下に一行改行してinsert modeになります.@<br>{}
`O` で上に一行改行になります。

=== セッション
現在の作業(開いているファイル、画面分割、タブ)状態を保存するためのセッション機能があります。@<br>{}
一旦PCを再起動するときなど、何かしらの理由で作業を中断せざるを得ない場合に使うと便利です。@<br>{}
@<code>{:mksession xxx.vim}でセッションファイルを保存します。@<br>{}
保存したセッションファイルを@<code>{vim -S xxx.vim}で復元することができます。@<br>{}
もしくはvimを開いた状態なら@<code>{:source xxx.vim}で復元できます。

=== 単語を修正
`IsGorilla` という関数名があって、それを `IsHuman` に修正したい場合、@<br>{}
カーソルをGにおいた状態でcwでGorillaを削除してinsert modeになるのでそこでHumanを入力することでサクッと修正できます。

=== 単語検索
検索したい単語にカーソルを当てた状態で開く`*`でファイル検索できます。@<br>{}
nで次、Nで前の検索結果にジャンプします。

=== 画面分割
@<code>{:vs}で水平分割、@<code>{:sp}で垂直分割できます。@<br>{}
また、@<code>{:vsp gorilla.go}というふうに引数を渡すことで分割したウィンドウで指定したファイルを開くことができます。

== おまけ
=== オススメvimrc設定
これは入れておけという設定をすこし紹介していきます。
設定したらもっと便利になります。

//emlist[vimrc][vimscript]{
" シンタックスを有効にする
syntax enable

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
//}

=== オススメプラグイン
個人的にオススメのプラグインを紹介します。@<br>{}
以下の設定を$HOME/.vim/dein.tomlに追加して、vimを再起動してください。

//emlist[go][toml]{
# Goを書くならこれ一択
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

# ステータスバーをかっこよくしてくれる
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


== まとめ
本章ではvimでGoを快適に書くためについて紹介しました。
まだまだvimにはたくさん便利な機能がありますので、気になる方はぜひ調べてみてください。

vimは慣れてしまえばとても快適にコーディングすることができますので、
ぜひ慣れてコーディングスピードを上げていきましょう。
