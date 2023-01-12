# hw1 report

|Field|Value|
|-:|:-|
|Name|成文瑄|
|ID|0716004|

## How much time did you spend on this project

e.g. 2 hours.

總共大約12小時

## Project overview

Describe the project structure and how you implemented it.

這個project的code，可以分成三部份。第一部份是一些include、define和一些宣告的地方；第二部份是寫出RE，然後告訴它當match到這個RE的時候要做什麼事；第三部份就是main function和一些在第一部份被宣告的function body的地方。我在implement這個project的時候用了三個state，分別是initial、comment和string，在comment會處理/**/這種形式的comment，當在intial看到/*就會進入comment state，在這個state會忽略之後看到的character直到看到*/回到initial state為止。而在initial state看到"而且look ahead看到後面有可以在字串中放的東西後又看到"會進入string state，在這個state為了要處理看到""只輸出"的情況，所以我為""和其他的character寫了兩條rule，就是在每看到一個字元的時候就印出那個字元（除了看到""的時候要印"），而不是一整個字串都match了才印，最後看到"再回到initial state。在initial state就是處理這兩個情況以外的所有東西，implement方式就是照著spec上的說明，把它寫成RE之後，然後輸出token。然後我們寫的這個scanner.l檔，lex會幫我們編出一個.c檔，再經過c的compiler編譯之後，會輸出一個執行檔。執行這個檔案，然後給它p語言的code，它就會用我們寫的RE去match這些code，然後切出token了。

## What is the hardest you think in this project

Let us know, if any.

最難的部份應該是debug，因為有時候我寫出來的RE運作的其實和我想的不一樣，所以在debug的時候很困難。

## Feedback to T.A.s

> Please help us improve our assignment, thanks.

我覺得作業的說明寫得很詳細，然後docker打包下來、code寫好後幾乎就可以直接run很方便，我覺得很棒。
