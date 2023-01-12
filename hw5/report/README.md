# 作業 5 報告

> 學生：成文瑄
>
> 學號：0716004

## 作業 5 心得

這次的作業在寫之前一直感覺沒有很難，因為像是老師在上課的時候也說不會很難，spec上也問說會不會太簡單之類的，又聽說有人兩、三小時就寫完了，讓我帶著期待的心情想說這次作業會愉快的寫完。結果自己寫的時候，寫了超久，覺得根本沒有說的這麼簡單，還是有很多需要注意的小細節（甚至還開始覺得是不是自己太廢了嗚嗚嗚）。
像是在分配給變數記憶體位置的時候，考慮到變數可能會有在不同層symbol table但有一樣的名字，所以用了map來記錄，map的key是變數的名字，value是一個stack。

    std::map<std::string, std::stack<int>> hash_addr;

每次變數被宣告的時候就把它的位置push進去。

    if(hash_addr.find(entry->getName())!=hash_addr.end()){
        hash_addr[entry->getName()].push(local_addr);
    }
    else{
        std::stack<int> addr_stack;
        addr_stack.push(local_addr);
        hash_addr[entry->getName()] = addr_stack;
    }
等離開這一層的symbol table時，就把這層用到的變數的位置都從stack pop出一個

    if(hash_addr.find(entry->getName())!=hash_addr.end()){
        if(hash_addr[entry->getName()].size()==1){
            hash_addr.erase(entry->getName());
        }
        else{
            hash_addr[entry->getName()].pop();
        }
    }

然後要使用變數的位置時，都使用stack最上面的那個。

    int addr;
    addr=hash_addr[entry->getName()].top();

另外一個遇到的比較麻煩的地方是label，一開始寫的時候是每次給一個label，用到之後就把編號++，jump的地方直接把跳去的地方寫死，但之後發現會有迴圈之內還有迴圈的問題，這樣它在jump的時候就沒辦法順利跳到正確的地方，所以後來也是用stack，然後每次給3個label（因為一個迴圈、if需要三個label)。

    label_base.push(label);
    label += 3;
    label_now = label_base.top();

stack記錄的是這層迴圈或是if的label的base，jump要跳去的地方變成相對於label base要加多少，等離開迴圈後把stack的東西pop一個出來。

    dumpInstrs("L%d:\n", label_now + 2);

    label_base.pop();
    if(label_base.size()!=0)
        label_now = label_base.top();
    else
        label_now = label; 


我覺得這次作業會讓我覺得困難的地方就是在於一開始在寫的時候自己可能有想到有哪些地方要注意，但看了測資之後以為這個要注意的地方不會發生（可能也和這次spec沒有像之前那麼詳細有關係），就想說先不管然後先寫寫看，然後過了就以為事情沒了。結果到後面才發現這件事要考慮，就得把原本寫的一直加東西或是做很大的改動，就會花很多時間。例子想是我在寫binary operator的時候，一開始在spec的最上面寫只考慮加減乘除和mod，我就天真的以為真的只有這幾個要寫（因為我本來還想過那些大於、小於之類的要不要寫），但後來寫到if、while的時候發現那些大於小於也要寫（但寫法和在assignment右邊的加減乘除又不同），更後面等到寫boolean的時候就發現我最一開始考慮的那些會在assignment右邊出現的那些大於小於等等都要寫了，然後這個時候改就變的很麻煩。所以如果能像之前的作業一樣在該寫那個東西的時候一次全部完整的寫完應該會好一點qq


## 整學期編譯器作業心得

寫完整學期編譯器作業之後，我現在的心情覺得好痛苦。就是雖然第一、二次作業沒有那麼困難，寫起來也還算開心（？），但經歷了後面第三、四、五次作業，現在整個回憶都被之前每天都在寫編譯器作業，寫了很久分數才好不容易多了一點點的辛苦給覆蓋。但如果扣除我自己這些主觀的心情的話，編譯器作業真的是我第一個寫了這麼多code、完成了一個大程式的作業，也覺得在寫作業的過程中有學到很多東西，每次拿助教公佈的code來改時，見識到了很多以前從來沒用過、也不知道的寫法和用法，原本很不懂指標、c的字串的我，現在稍微不像以前那麼害怕了，也覺得好像比較懂它們了，而且在寫作業的過程中，有一些老師上課提到的觀念，也是在真的自己寫時才好好的想過一遍弄清楚，而且也了解了從0開始建立一個編譯器的過程。
