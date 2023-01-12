# hw2 report

|||
|-:|:-|
|Name|成文瑄|
|ID|0716004|

## How much time did you spend on this project

e.g. 2 hours.

總共大約3~4小時

## Project overview

Describe the project structure and how you implemented it.

我在這個project中做的事可以分成兩部份，第一部份是要在scanner中return token給parser用（透過include "parser.h"），第二部份是要在parser中宣告那些會用到的token，還有寫production rule。宣告token的時候我除了單純宣告它們之外，也用了left來處理operator的associativity（left由上而下是優先權低到高，left表示都是left associativity）。宣告好token之後，就是用這些token然後按照著spec上的敘述寫出production rule，比較要注意的地方是像id或是arg的list，我會分成類似空字串｜non_empty_list兩部份來寫，寫的時候一開始沒注意讓他們都產生了一樣的sentence，就產生了一些conflict（雖然答案還是對的），之後才把它們消除掉。所以執行時parser就會一直和scanner要下一個token，去match production rule，可以reduce回start symbol就表示文法是對的!

## What is the hardest you think in this project

Let us know, if any.

最難的部份應該是取token名字的部份吧qq 一開始我把那些keyword的return token直接就取為它們的名字（ 像是keyword中的begin就直接取名為BEGIN、end就取為END），結果沒想到這樣直接就踩到了一個大雷點，就是BEGIN好像是yacc已經有功用的字了，所以就導致我一開始在make的時候出現了很多很多看起來不是我的錯的error，之後是問了剛好也發生這件事的朋友才知道原來是這樣！

## Feedback to T.A.s

> Please help us improve our assignment, thanks.

作業真的設計的很好～
