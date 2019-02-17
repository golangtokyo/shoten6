# タイトル未定

## この本のビルドの仕方

### Dockerを使う

TeXの環境構築が困難な場合、一式セットアップ済みの[dockerイメージ](https://registry.hub.docker.com/u/vvakame/review/)を用意してあるので使ってください。
Dockerがうまく動くようになっている場合、以下のコマンドで細かい準備なしにビルドを行うことができます。

```
$ docker pull vvakame/review
$ ./build-in-docker.sh
```

### HTML出力

`npm run web` を実行すると、`publish/`ディレクトリ以下に公開用HTMLファイルおよびcss,画像を出力します。

## ライセンス等の表記

設定ファイル、テンプレートなど制作環境（techbooster-doujin.styなど）TechBoosterのものを使用しています。
