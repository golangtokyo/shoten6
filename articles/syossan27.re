={syossan27_title} GoとKafkaを用いた分散メッセージシステム入門

== はじめに
株式会社ミクシィでエンジニアをしているsyossan27@<fn>{syossan27}です。
Goのシンプルな思想に惚れ込み、Goあんこ@<fn>{gounco}という勉強会の主催をやっています。

//footnote[syossan27][@<href>{https://twitter.com/syossan27}]
//footnote[gounco][@<href>{https://gounconference.connpass.com/}]

本章では、分散メッセージングミドルウェアであるKafkaをGoを通して学んでいきます。
簡単なKafkaの説明から始め、KafkaとGoを用いて簡単なシステムを作ってみるところまでをゴールとします。

== Kafkaとは

Kafka（@<href>{https://kafka.apache.org/}）とは、LinkedIn@<fn>{linkedin}で開発された分散メッセージングミドルウェアです。
背景として、LinkedIn上での大量のアクティビティデータを機械学習やフィード画面に表示するためのデータパイプラインとして以下を目的として開発されました。

 * 高スループットによるリアルタイムストリーミング
 * パフォーマンスを損なわないメッセージストア
 * 耐障害性を保つための分散型システム
 * メッセージロストの防止

開発以前に、ActiveMQ@<fn>{activemq}を使った本番環境での運用を試してみたようなのですが、メッセージキューがメモリ容量を超えると急激なパフォーマンス低下を起こしたり、負荷分散のアプローチが多数の問題により頓挫したりとかなりの苦労があったようです@<fn>{linkedin-paper}。

//footnote[linkedin][@<href>{https://ja.wikipedia.org/wiki/LinkedIn}]
//footnote[activemq][@<href>{http://activemq.apache.org/}]
//footnote[linkedin-paper][1.2 Struggles with real-time infrastructureに記述: @<href>{http://sites.computer.org/debull/A12june/pipeline.pdf}]

=== Kafkaの基礎知識

Kafkaには分散メッセージングの仕組みとしてBroker、Topic、Partition、Offsetという要素があります。それぞれ詳しく見ていきましょう。

==== Broker

Brokerは１つのサーバにつき１つ存在する要素で、メッセージの受信・配信を担います。Brokerを稼働させているサーバ群をクラスタリングすることで、メッセージングシステムの可用性を上げることが出来ます。実際に本番運用する際には必ずクラスタリングを行うようにしましょう。

==== Topic

Topicはメッセージを種別する要素で、例えばlog.createdという名前であればログの生成に関するメッセージを格納するTopicとして扱います。TopicはBrokerを構成する要素で、Broker内に１つから複数Topicまで存在します。メッセージの受信、配信先としてBrokerと併せて指定されます。

==== Partition

PartitionはTopicを更に分割する要素で、１つのTopicに対する膨大なメッセージのやり取りを分散させることで効率的に受信・配信することができます。

==== Offset

OffsetはPartitionに内包される要素で、Partitionで受信したメッセージにはそれぞれ連番となるOffset番号と呼ばれるものが割り当てられており、それをそれぞれのPartitionで位置情報として管理しています。
Offsetには３種類存在し

 * Log End Offset : Partitionの末尾を表す
 * Current Offset : Consumerによって読み込まれた場所を表す
 * Commit Offset : メッセージがCommitされた場所を表す

というものがそれぞれあります。ここでCommit Offsetだけが分かりにくい概念かと思いますが、「ここまでは確実に読み込まれましたよ」ということを確定しているOffsetになります。

ここまでのイメージをまとめると@<img>{syossan27-kafka-detail}のようになります。

//image[syossan27-kafka-detail][イメージ図]{
//}

また、Kafka以外の重要な要素としてZooKeeper@<fn>{zookeeper}というミドルウェアがあります。ZooKeeperは分散システムにおける設定情報やリソースの一元管理を担い、Kafkaでは主にTopicの構成（Partition数、Replicasのパスなど）の保持や、クラスタ内のブローカーのリストを保持等をしています。

//footnote[zookeeper][@<href>{https://zookeeper.apache.org/}]

=== Kafkaで何が出来るのか？

このように既存のメッセージングミドルウェアが実現出来なかった機能を持ち合わせているKafkaですが、一体Kafkaを使って何が出来るのでしょうか？  
Kafkaの公式ページで紹介されているユースケースをもとに幾つか紹介していきます。@<fn>{Kafka-usecase}

==== メッセージング

ActiveMQやRabbitMQ@<fn>{rabbitmq}のようにKafkaはオーソドックスなメッセージキューイングのブローカーとして使うことが出来ます。
Kafkaは高スループットかつ耐障害性において他のメッセージングミドルウェアに勝るため、秒間のメッセージ量が多い大企業では好んで使われます。
ただし、Kafkaは耐障害性を高めるため、クラスタ内の各ブローカーへ向けたメッセージ複製という機構を持つため、どうしてもそれを持たないミドルウェアにはスループットが敵わない面を持ちます。
ですので、障害時にメッセージをある程度ロストしても構わない、かつ大量のメッセージを捌くスループットが欲しいという場合にはレプリケーション機構を持たないNATS@<fn>{nats}や、ブローカーレスであるnanomsg@<fn>{nanomsg}、ZeroMQ@<fn>{zeromq}に軍配が上がります。

==== アクティビティトラッキング

元々LinkedInでアクティビティトラッキングのために開発されたため、勿論ですがアクティビティトラッキングに用いられることも多々あります。
アクティビティトラッキングはユーザー数やトラッキング種別数にも依りますが、多量のデータを捌く必要性があるため高スループットが求められます。
今まで述べた通り、Kafkaは高スループットを旨としておりLinkedInでは2012年の段階でピーク時に秒間17万メッセージをも捌いている実績があります@<fn>{linkedin-paper2}。
現在のバージョンではさらなるパフォーマンス改善が施されているため、もっとメッセージを捌くことが出来ることでしょう。

また、メッセージをHadoopなどデータウェアハウスへロードするためのKafka Connectと呼ばれるコネクタが用意されているのも、好んで使われる一因だと思います。

==== ストリーム処理

ストリーム処理とは、増大し続けるデータを永続的に処理し続けることを指し、主に幾つかの処理ステップをそれぞれトピックに分け、処理を行い次のトピックへ送信といった形で順々にデータ加工をしていく流れを取ります。
また、Kafkaのv0.10からKafka Streamsというライブラリが同梱され、これを用いることで上記の処理の流れをアプリケーションで簡単に実現できるようになりました。

==== イベントソーシング

イベントソーシングとは、イベントを永続化させ必要に応じてリプレイすることで状態を作り出すアーキテクチャで、永続させるイベントストアとしてKafkaを用いる事ができます。  
注意点として、リプレイに必要なデータを保持するtopicの永続化に伴う適切なLog Compaction設定や、イベントをconsumeするtopic・リプライの中間データ用topicの適切な分割・設定などがあります。

//footnote[Kafka-usecase][@<href>{https://Kafka.apache.org/uses}]
//footnote[rabbitmq][@<href>{https://www.rabbitmq.com/}]
//footnote[nats][@<href>{https://nats.io/}]
//footnote[nanomsg][@<href>{https://github.com/nanomsg/nanomsg}]
//footnote[zeromq][@<href>{http://zeromq.org/}]
//footnote[linkedin-paper2][2.2 Kafka Usage at LinkedInに記述: @<href>{http://sites.computer.org/debull/A12june/pipeline.pdf}]

== Go × Kafka

さて、ここからは実際にKafkaとGoを組み合わせていきましょう。  
まず最初に読者の方が気になるのは「KafkaとGoの親和性ってどうなの？」というところだと思います。  
筆者は実際にKafkaとGoを使ったシステムを構築する中で、かなり親和性は高いのでは？と考えております。  
勿論、ベターな選択肢としてはJavaやScalaなどのJVM言語を使うのが賢明であると思いますが（clientが公式でサポートされているため）、Goに関しては非公式ライブラリであってもKafkaのバージョンアップについていけないということも無く、また言語特性として並列処理に長けているという点も多重Consumerでの処理の書きやすさに繋がっていると感じます。

そんなGoとKafkaを使い、簡単なメッセージングシステムを組みながら分散メッセージの世界に入っていきましょう。バージョンはそれぞれGo v1.12、Kafka v2.2.0を用いて作成します。

=== 作成するシステムについて 

今回は@<img>{syossan27-system-image}のようなイメージのシステムを構築していきたいと思います。

//image[syossan27-system-image][イメージ図]{
//}

APIサーバ兼Producerの役割を持つアプリケーションを前段に置き、リクエストを受けたエンドポイント毎に対応したKafka BrokerのTopicへメッセージを送信します。そしてTopicからメッセージを受け取るConsumerを作成し、ログにメッセージを受け取ったことを出力するところまで作っていきましょう。

=== ライブラリ選定

GoでKafkaを扱うライブラリは幾つか存在します。
現在も活発に更新されている代表的なライブラリは以下の３つです。

 * github.com/Shopify/sarama@<fn>{sarama}
 * github.com/confluentinc/confluent-kafka-go@<fn>{confluent-kafka-go}
 * github.com/segmentio/kafka-go@<fn>{kafka-go}

saramaはこの３つの中では一番Star数が多く、またポピュラーに使われているライブラリです。筆者もsaramaを用いてシステム開発を行いましたが、特に問題もなく動きました。またShopify, IBM, Herokuなど大手が使っていることからも信頼性が伺えます@<fn>{sarama-used}。
confluent-kafka-goはlibrdkafkaというKafkaのCライブラリのラッパーという立ち位置になります。なので、このライブラリを使用する際にはlibrdkafkaのインストールが必要になりますが、それが原因でwindowsでのサポートはconfluent-kafka-goではされていないようです。
kafka-goは、他２つのライブラリより後発のライブラリで、saramaやconfluent-kafka-goの問題点を解決することを目的として開発されました。しかし、現状Kafkaの最新バージョンに追従していないように見受けられるので、他のライブラリを使うのが得策かと思います。

今回はsaramaを使って構築していきます。saramaの注意点として、以前まではsarama-cluster@<fn>{sarama-cluster}というライブラリも並行して使用しないとKafkaのConsumer Groupという概念を扱うことが出来なかったのですが、2018/09/28にsarama v1.19.0@<fn>{implement-consumer-group}でsarama-clusterの機能がsaramaに合併されました。ですので、古い技術記事にはsarama-clusterを使ってくださいと記載されている場合があるかもしれないのでご注意ください。

//footnote[sarama][@<href>{https://github.com/Shopify/sarama}]
//footnote[confluent-kafka-go][@<href>{https://github.com/confluentinc/confluent-kafka-go}]
//footnote[kafka-go][@<href>{https://github.com/segmentio/kafka-go}]
//footnote[sarama-used][@<href>{https://github.com/Shopify/sarama/wiki/Frequently-Asked-Questions#is-sarama-used-in-production-anywhere}]
//footnote[sarama-cluster][@<href>{https://github.com/bsm/sarama-cluster}]
//footnote[implement-consumer-group][@<href>{https://github.com/Shopify/sarama/releases/tag/v1.19.0}]

=== Brokerの起動

それでは、まずはKafka Brokerを立ち上げていきましょう。Kafka公式サイトのQuick Start（@<href>{https://kafka.apache.org/quickstart}）を見てBrokerを立ち上げるのも一つですが、wurstmeister/kafka-docker@<fn>{kafka-docker}という便利なプロダクトがあるのでこれを使っていきましょう。

事前準備としてDocker・Docker Composeのインストールを行っておいてください。

次にkafka-dockerの環境構築をしていきます。

//list[clone kafka-docker][kafka-dockerのclone][]{
% git clone git@github.com:wurstmeister/kafka-docker.git
% cd kafka-docker
//}

さて、ここでdocker-compose upで立ち上げたいところですが、その前に１点だけdocker-compose.ymlのKAFKA_ADVERTISED_HOST_NAMEに修正を加えます。

//listnum[docker-compose.yml][Docker Compose設定][yaml]{
version: '2'
services:
  zookeeper:
    image: wurstmeister/zookeeper
    ports:
      - "2181:2181"
  kafka:
    build: .
    ports:
      - "9092"
    environment:
      // KAFKA_ADVERTISED_HOST_NAME: 192.168.99.100
      KAFKA_ADVERTISED_HOST_NAME: [HOST_IP] // ifconfigを実行して取得したホストIPを入力してください
      KAFKA_ZOOKEEPER_CONNECT: zookeeper:2181
    volumes:
      - /var/run/docker.sock:/var/run/docker.sock
//}

これで準備が出来ましたので立ち上げてみましょう。今回は３つのKafka Brokerを立ち上げます。

//list[start-docker-compose][Docker Composeの立ち上げ][]{
% docker-compose up -d --scale kafka=3
Creating kafka-docker_zookeeper_1 ... done
Creating kafka-docker_kafka_1     ... done
Creating kafka-docker_kafka_2     ... done
Creating kafka-docker_kafka_3     ... done
//}

作成されたkafka-docker_kafka_XがBrokerになり、kafka-docker_zookeeper_1が各Brokerが使用しているzookeeperになります。それでは正常に動いていることを確認するために、簡単にコマンドライン上でProducer・Consumerを動かしてメッセージのやり取りをしてみましょう。まずはProducer・Consumerのコマンドラインツールを取得するためにKafkaのコードを取得してきます。2019年3月現在ではKafka 2.2.0が最新バージョンになりますので、そちらに準拠して進めていきます。

//list[download-kafka][Kafkaのダウンロード][]{
% wget http://ftp.jaist.ac.jp/pub/apache/kafka/2.2.0/kafka_2.12-2.2.0.tgz
% tar -xzf kafka_2.12-2.2.0.tgz
//}

それでは最初にtopicsの作成、その後Producerを実行します。実行に際してKafka Brokerのポート番号が必要になりますので、kafka-dockerで立ち上げたKafka Brokerの情報を取得してから実行します。

//list[kafka-console-producer][Producerの実行][]{
% cd kafka-docker
% docker-compose ps
          Name                                 Ports
---------------------------------------------------------------------------
kafka-docker_kafka_1     0.0.0.0:32801->9092/tcp
kafka-docker_kafka_2     0.0.0.0:32802->9092/tcp
kafka-docker_kafka_3     0.0.0.0:32803->9092/tcp
kafka-docker_zookeeper_1 0.0.0.0:2181->2181/tcp, 22/tcp, 2888/tcp, 3888/tcp

// test topicの作成。今回はBrokerを３つ動かしているため、Replicas・Partitionsともに３を設定しています。
% ./kafka-topics.sh --create \
    --zookeeper localhost:2181 \
    --replication-factor 3 \
    --partitions 3 --topic test

// test topicの情報を確認
% ./kafka-topics.sh --describe --zookeeper localhost:2181 --topic test
Topic:test	PartitionCount:3	ReplicationFactor:3	Configs:
Partition: 0 Leader: 1002 Replicas: 1002,1001,1003 Isr: 1002,1001,1003
Partition: 1 Leader: 1003 Replicas: 1003,1002,1001 Isr: 1003,1002,1001
Partition: 2 Leader: 1001 Replicas: 1001,1003,1002 Isr: 1001,1003,1002

% ./kafka-console-producer.sh \
    --broker-list localhost:32801,localhost:32802,localhost:32803 \
    --topic test
//}

次に別プロセスでConsumerを実行してみましょう。

//list[kafka-console-consumer-1][Consumerの実行][]{
% ./kafka-console-consumer.sh \
    --bootstrap-server localhost:32801,localhost:32802,localhost:32803 \
    --topic test --from-beginning
//}

これでtest topicを介したメッセージングの準備は出来ました。Producerを実行しているプロセスで何か文字を打ってみましょう。Consumerを実行しているプロセスにProducerで打った文字が表示されればKafka Brokerが正常に動いている証です。

//footnote[kafka-docker][@<href>{https://github.com/wurstmeister/kafka-docker}]

=== Producerの作成

ここからはGoの出番です。@<list>{kafka-console-producer}で実行していたProducerに当たる部分を実装していきます。まずは@<list>{SyncProducer}を見ていきましょう。

//listnum[SyncProducer][Kafka SyncProducer][go]{
package main

import (
	"flag"
	"fmt"
	"log"

	"github.com/Shopify/sarama"
)

func main() {
	// 接続先となるKafka Brokerをコマンド引数から取得
	flag.Parse()
	brokerList := flag.Args()

	// Producer設定
	config := sarama.NewConfig()

	// Kafka Brokerにへのメッセージ送信が成功したかどうかの条件を設定
	//   WaitForAll：
  //     送信メッセージがmin.insync.replicasで設定されたBrokerの数だけレプリケーションが成功した場合に成功とする
	// 	 WaitForLocal：
  //     送信メッセージがリーダーとなっているBrokerに書き込みされた場合に成功とする
	// 	 NoResponse：
  //     送信メッセージが書き込みされたか否かに関わらず成功とする
	config.Producer.RequiredAcks = sarama.WaitForAll

	// メッセージ送信の再試行回数（デフォルトは3）
	config.Producer.Retry.Max = 5

	// 有効にするとメッセージ送信成功時にSuccesses channelに対してデータが送信される
	// SyncProducerを使用する場合には有効が必須
	config.Producer.Return.Successes = true

  // 有効にするとメッセージ送信失敗時にErrors channelに対してデータが送信される
	// SyncProducerを使用する場合には有効が必須（デフォルトは有効）
	config.Producer.Return.Errors = true

	// Producerの作成
	producer, err := sarama.NewSyncProducer(brokerList, config)
	if err != nil {
		log.Fatalln("failed create producer", err)
	}
  defer producer.Close()

	// メッセージの送信
	partition, offset, err := producer.SendMessage(
		&sarama.ProducerMessage{
			Topic: "test",
			Value: sarama.StringEncoder("Hello World"),
		},
	)
	if err != nil {
		log.Fatalf("failed send message: %s\n", err)
	}

	fmt.Printf("Partition: %d, Offset: %d\n", partition, offset)
}
//}

@<list>{SyncProducer}はコマンド引数として渡したBrokerのアドレスに対して、「Hello World」というメッセージをtest topicに送信する簡単なものになっています。実装としてはProducerのconfigを諸々設定し、Producerを作成・メッセージの送信という流れになっており、そこまで難しくないでしょう。
さて、それでは試しにこちらのコードを動かしてConsumerにメッセージが届くかどうか見てみましょう。先程と同じようにProducerとConsumerは別々のプロセスで立ち上げることに注意してください。

//list[run-producer][Producerの実行][]{
% go run producer.go localhost:32801,localhost:32802,localhost:32803

Partition: 2, Offset: 0
//}

//list[kafka-console-consumer-2][Consumerの実行][]{
% ./kafka-console-consumer.sh \
    --bootstrap-server localhost:32801,localhost:32802,localhost:32803 \
    --topic test --from-beginning

Hello World
//}

いかがだったでしょうか？ひとまずはProducerが動くことが体験出来ましたね。さて、このサンプルコードで扱っているProducerは一口にProducerと言っても「同期的に実行されるProducer（SyncProducer）」になります。しかし、saramaの公式ドキュメントにはSyncProducerを「効率的でなく、メッセージをロストする可能性があります」と評しており、あまり推奨されているProducerではないようです。そこでもう一つのProducerである「非同期に実行されるProducer（AsyncProducer）」で実装してみましょう。

//listnum[AsyncProducer][Kafka AsyncProducer][go]{
package main

import (
	"flag"
	"fmt"
	"log"

	"github.com/Shopify/sarama"
)

func main() {
	// 接続先となるKafka Brokerをコマンド引数から取得
	flag.Parse()
	brokerList := flag.Args()

	// Producer設定
	config := sarama.NewConfig()
	config.Producer.RequiredAcks = sarama.WaitForAll
	config.Producer.Retry.Max = 5
	config.Producer.Return.Successes = true

	// Producerの作成
	producer, err := sarama.NewAsyncProducer(brokerList, config)
	if err != nil {
		log.Fatalln("failed create producer", err)
	}
	defer producer.Close()

	// メッセージの送信
	message := &sarama.ProducerMessage{
		Topic: "test",
		Value: sarama.StringEncoder("Hello World"),
	}

	producer.Input() <- message

	select {
		// メッセージ送信成功時処理
		case result := <-producer.Successes():
			fmt.Printf("Partition: %d, Offset: %d\n",
 				result.Partition, result.Offset)
		// メッセージ送信失敗時処理
		case err := <-producer.Errors():
			log.Fatalf("failed send message: %s\n", err)
	}
}
//}

SyncProducerとはまた少し違った実装になりましたね。このようにAsyncProducerはChannelを使った実装が主になります。AsyncProducerのコードを元に、次はAPIの実装を付け加えていきましょう。

//listnum[producer][API実装の追加][go]{ 
package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"
	"net/url"
	"strings"

	"github.com/Shopify/sarama"
)

func main() {
	// 接続先となるKafka Brokerをコマンド引数から取得
	flag.Parse()
	brokerList := flag.Args()

	// Producer設定
	config := sarama.NewConfig()
	config.Producer.RequiredAcks = sarama.WaitForAll
	config.Producer.Retry.Max = 5
	config.Producer.Return.Successes = true
	config.Producer.Return.Errors = true

	// Producerの作成
	producer, err := sarama.NewAsyncProducer(brokerList, config)
	if err != nil {
		log.Fatalln("failed create producer", err)
	}
	defer producer.Close()

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
    // URLクエリパラメータの取得
		query, err := url.ParseQuery(r.URL.RawQuery)
		if err != nil {
			w.WriteHeader(500)
			fmt.Printf("failed query parse: %s\n", err)
			return
		}

		messageContent, ok := query["message"]
		if !ok {
			w.WriteHeader(500)
			fmt.Println("not found message query")
			return
		}

		// メッセージの送信
		message := &sarama.ProducerMessage{
			Topic: "test",
			Value: sarama.StringEncoder(
				strings.Join(messageContent, " "),
			),
		}

		producer.Input() <- message

		select {
		// メッセージ送信成功時処理
		case result := <-producer.Successes():
			fmt.Printf("Partition: %d, Offset: %d\n", 
				result.Partition, result.Offset)
		// メッセージ送信失敗時処理
		case err := <-producer.Errors():
			w.WriteHeader(500)
			fmt.Printf("failed send message: %s\n", err)
		}
	})

	log.Fatal(http.ListenAndServe(":8080", nil))
}
//}

今回はこのように送信したいメッセージをmessageというURLクエリパラメータを付加してリクエストを送るような実装にしています。それでは試しにHello World以外の何かメッセージを送ってみましょう。

//list[send-message][メッセージの送信][]{
% go run producer.go localhost:32801,localhost:32802,localhost:32803
% curl -X GET http://localhost:8080\?message=hello
//}

これでconsumerで確認するとhelloが表示されることでしょう。さて、ここまででAPIサーバ兼Producerの役割を持つアプリケーションは完成しました。本節の最後に今回紹介した以外のProducerでの設定項目について触れていきたいと思います@<fn>{sarama-producer-config}。

//list[producer-config][Producer設定一覧][]{
// メッセージの圧縮形式を設定
// gzip, snappy, lz4, zstdから選択が可能
// 筆者のオススメはzstdです。高圧縮・高スループットのため、他の圧縮形式よりも優秀です
// 尚、圧縮形式を設定する際にはBrokerのcompression.typeも併せて設定することを忘れないようにしましょう
config.Producer.Compression = sarama.CompressionZSTD

// メッセージの圧縮レベルを設定
// 圧縮レベルは各圧縮形式によって異なります
//   gzip : 1 ~ 9 （大きい数字ほど高圧縮）
//   snappy : none （snappyには圧縮レベルが存在しません）
//   lz4 : 1 ~ 17
//   zstd : 131072 ~ 22
config.Producer.CompressionLevel = 3

// メッセージをBrokerへ送信する頻度を設定
// デフォルト設定でも遅延なくベストエフォートでBrokerへメッセージを送信しようとするので、あまり気にせずともOK
// 送信のトリガーとなるバイト数
config.Producer.Flush.Bytes

// 送信のトリガーとなるメッセージ数
config.Producer.Flush.Messages

// 送信の間隔時間
config.Producer.Flush.Frequency

// １回の送信時に送るメッセージ数
config.Producer.Flush.MaxMessages

// メッセージを必ず１度だけ送信することを保証するかどうか設定できます
config.Producer.Idempotent = true

// 有効化する際には以下も併せて設定してください
// 未処理のメッセージの最大保持数
config.Net.MaxOpenRequests = 1 
// kafkaのバージョン
config.Version = sarama.V2_1_0_0

// メッセージの送信最大バイト数
config.Producer.MaxMessageBytes

// どのPartitionに対してメッセージを送信するかアルゴリズムの設定
config.Producer.Partitioner

// RequiredAcksの応答待機時間の設定
config.Producer.Timeout
//}

様々な設定がありますが、プロジェクトの規模が物凄く大きいなど特別な状況以外は特に設定することは無いものが殆どかと思われます。メッセージの量にも依るのですがKafkaのBrokerにログファイルが溜まり過ぎて、ディスク容量を食い潰して突然の死を迎えるという状況を目にしたことがあるので、筆者としてはメッセージを圧縮しておいた方が良いと思います。

//footnote[sarama-producer-config][Producer structに記載@<href>{https://godoc.org/github.com/Shopify/sarama#Config}]

=== Consumerの作成

次にConsumerも作成していきましょう。

//listnum[consumer][Kafka Consumer][go]{
package main

import (
	"context"
	"flag"
	"fmt"
	"github.com/Shopify/sarama"
	"log"
	"time"
)

// ConsumerGroupHandler Interfaceを満たすために、Setup・ConsumeCalim・CleanupメソッドはシグネチャをInterfaceに合わせています
type exampleConsumerGroupHandler struct{}

// ConsumerClaim処理の前に実行される処理
func (exampleConsumerGroupHandler) Setup(_ sarama.ConsumerGroupSession) error {
	return nil
}

// consume処理、goroutineで並行処理されるため共通データの読み取り・書き込みを行うときは排他制御を行うなどの施策が必要
func (h exampleConsumerGroupHandler) ConsumeClaim(
	sess sarama.ConsumerGroupSession,
	claim sarama.ConsumerGroupClaim,
) error {
	for msg := range claim.Messages() {
		fmt.Printf("Message: %d\n", string(msg.Value))
		fmt.Printf("Partition: %d, Offset: %d\n", msg.Partition, msg.Offset)
		sess.MarkMessage(msg, "")
	}
	return nil
}

// goroutineで動いているConsumerClaim処理が全て終了し、offsetがcommitをする前に実行される処理
func (exampleConsumerGroupHandler) Cleanup(
	_ sarama.ConsumerGroupSession,
) error { return nil }

func main() {
	// 接続先となるKafka Brokerをコマンド引数から取得
	flag.Parse()
	brokerList := flag.Args()

	// Consumer設定
	config := sarama.NewConfig()

	// 有効にするとメッセージ受信中に失敗したデータがErrors channelに対して送信される（デフォルトは無効）
	config.Consumer.Return.Errors = true

	// メッセージ受信を開始するoffsetのアルゴリズムの設定
	//   OffsetNewest : 最新のoffsetからconsumeを開始し、新着のメッセージを受け取ります
	//   OffsetOldest : 最古のoffsetからconsumeを開始し、古いメッセージから新着のメッセージまで受け取ります
	config.Consumer.Offsets.Initial = sarama.OffsetNewest

	// offsetの保持期間、0に設定されている場合はBrokerのoffsets.retention.minutesが適用されます（デフォルトは0）
	config.Consumer.Offsets.Retention = 10 * time.Second

	// Brokerからのレスポンスの待機時間
	config.Net.ReadTimeout = 60 * time.Second

	// kafkaのバージョン
	config.Version = sarama.V2_1_0_0

	// Consumer Groupの作成
 	consumer, err := sarama.NewConsumerGroup(
		brokerList,
		"consumer-group",
		config,
	)
	if err != nil {
		log.Fatalln("failed create consumer", err)
	}
	defer consumer.Close()

	// メッセージ受信失敗時処理
	go func() {
		for err := range consumer.Errors() {
			fmt.Printf("failed receive message: %s\n", err)
		}
	}()

	// メッセージの受信
	ctx := context.Background()
	for {
		topics := []string{"test"}
		handler := exampleConsumerGroupHandler{}

		// 指定したhandlerに紐付くメソッドを以下の定義に従って実行する
		//   1. partitionを均一にconsumeするように割り当て
		//   2. handler.Setupを実行
		//   3. handler.ConsumeClaimを実行
		//   4. handler.ConsumeClaimが終了
		//     またはcontextでのキャンセル、リバランス処理が実行される時にセッションを終了する
		//   5. offsetをcommitする前にhandler.Cleanupを実行
		//   6. offsetをcommitする
		err := consumer.Consume(ctx, topics, handler)
		if err != nil {
			panic(err)
		}
	}
}
//}

さて、この中で新たな概念としてConsumer Groupというものが出てきましたね。これは特定のConsumer Groupに所属するConsumerの一つに重複無くメッセージを届けるような時に使われます。殆どの場合でConsumerを多数用意し、複数のPartitionを上手くバランシングすると思われますので、基本的にはConsumer Groupで作成するのが良いでしょう。
基本的にはコメントで書いてあるように動くのですが、今回はSetup処理やCleanup処理は特に何もしない形で実装しております。本来であるなら、メッセージをconsumeした後によしなにデータベースへデータを挿入したりなどの処理が組み込まれますので、そういった時にHandlerで@<code>{sql.DB}を持ち、そこを通してデータベースのセットアップやクローズ処理をする際に使うのがベターでしょう。

それでは、Producerの時と同じく設定類についても軽く触れていきます@<fn>{sarama-consumer-config}。

//list[consumer-setting][Consumer設定一覧][]{
// consumerのセッションがアクティブになっているか確認する間隔時間
// デフォルトは3秒で、Consumer.Group.Session.Timeoutの1/3以下の値を設定するのが良い
config.Consumer.Group.Heartbeat.Interval = 3 * time.Second

// Consumer Group参加時に渡すUserDataの設定
// custom partition割り当てで扱うが、通常は特に気にしなくても良い
config.Consumer.Group.Member.UserData

// topic partitionにメンバーを割り当てるstrategyの設定（デフォルトはBalanceStrategyRange）
config.Consumer.Group.Rebalance.Strategy = sarama.BalanceStrategyRange

// リバランスが行われて、Consumer Groupに参加する時の許容時間（デフォルトは60秒）
config.Consumer.Group.Rebalance.Timeout = 60 * time.Second

// リバランスが行われて、Consumer Groupに参加する時の最大リトライ回数（デフォルトは4）
config.Consumer.Group.Rebalance.Retry.Max = 4

// リバランスの再試行の許容時間（デフォルトは2秒）
config.Consumer.Group.Rebalance.Retry.Backoff = 2

// この設定の時間内にハートビートを受信しなかった場合にエラーとし、ConsumerをConsumer Groupから除外するための設定（デフォルトは10秒）
config.Consumer.Group.Session.Timeout = 10 * time.Second

// Partitionの読み込み失敗から再試行するまでの待機時間（デフォルトは2秒）
config.Consumer.Retry.Backoff = 2 * time.Second

// Backoff時間の計算ロジックを埋め込むことが出来る
config.Consumer.Retry.BackoffFunc

// consumeするメッセージの最小バイト数
config.Consumer.Fetch.Min

// consumeするメッセージのデフォルトで確保するバイト数
config.Consumer.Fetch.Default

// consumeするメッセージの最大バイト数
config.Consumer.Fetch.Max

// トランザクション設定（デフォルトはReadCommitted）
//   sarama.ReadCommitted : commitされたメッセージのみconsumeする
//   sarama.ReadUncommitted : commitされていないメッセージもconsumeする
config.Consumer.IsolationLevel =  sarama.ReadCommitted

// メッセージをconsumeするための許容時間（デフォルトは100ミリ秒）
config.Consumer.MaxProcessingTime = 100 * time.Microsecond
//}

全ての設定を使うことは少ないと思いますが、こういった設定があるということを頭の片隅にでも覚えておいてください。

//footnote[sarama-consumer-config][Consumer structに記載@<href>{https://godoc.org/github.com/Shopify/sarama#Config}]

== おわりに

ここまで読んで頂きありがとうございます。分かりにくいところもあったかもしれませんが、Kafkaの様々な概念は実際に動かしてみないと分かりづらいことが多いです。是非とも個人開発や現場で使ってみてKafkaを極めていってください。また、Kafkaの情報源として最近発売された「Apache Kafka 分散メッセージングシステムの構築と活用」@<fn>{kafka-book}という本は今回扱ったような基本的な説明から、一歩進んだ実用的な例も書かれていますので一読の価値ありです。さらに、Confluent社のBlog@<fn>{confluent-blog}はKafkaの話題をよく取り扱っているのでRSS等でウォッチするのもオススメします。改めてになりますが、読んで頂きありがとうございました。

//footnote[kafka-book][@<href>{https://www.amazon.co.jp/dp/B07H8D6CL9}]
//footnote[confluent-blog][@<href>{https://www.confluent.io/blog}]
