﻿
おれおれJsonパーサちゃん
===============================================================================

Jsonパーサだよおおおおおおおおお！！！

まだ最適化もエラーハンドリングも仕込んでないよおおおおお！！！！

勝手に使って死んでも知らないよおおおおおおおお！！！

にほんごしかわからないのおおおおおおおおおおおおおおおおおおお

Feature of this project.
-------------------------------------------------------------------------------
__はやい。rapi○jsonを超えると豪語した。__でででできるし(震え声)

また作者のコンセプチュアルスキルの向上、プロジェクトを構築する練習の一環、
また初の公開リポジトリによる制作でもある。
つまり練習みたいなものなので使うな。勝手に使っても責任は取らない。使うなら一言言うこと。

連絡先: lowe4649+github♪gmail.com

♪→@。+githubを付けなかったら問答無用でスパム行きなので注意。あと作者は日本語しか喋れない。

To-Do (In order of preference)
-------------------------------------------------------------------------------
* バグ修正
* ストリーム対応
* インターフェイス再考
* エラーハンドリング
* 高速化
	- ハッシュ関数の実装
	- いろいろ
* ぽげええええええええええええええええええええええええええええええええ

対応済み

* ~~ファイル分割(いまさら)~~

Bug List
-------------------------------------------------------------------------------
* __Unicodeエスケープシーケンスを含む文字列がパースできない可能性がある__

修正済み

* ~~数値のパースが出来ない(0が返る)~~

Change Log (Only those related to the use)
-------------------------------------------------------------------------------
* 2014-03-07 数値のパースが出来ないバグを修正
* 2014-03-06 ライセンス記載
* 2014-03-06 メモリリークするバグを修正

License
-------------------------------------------------------------------------------
このライブラリ自体は未完成なのでまだライセンスは記述していません。
以下は使用している外部ライブラリのライセンスです。

### Boost C++ Library
> Boost Software License - Version 1.0 - August 17th, 2003

> Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

> The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

### Boost.Predef 1.0
> Rene Rivera

> Copyright © 2005 Rene Rivera

> Copyright © 2008-2013 Redshift Software Inc

> Distributed under the Boost Software License, Version 1.0.