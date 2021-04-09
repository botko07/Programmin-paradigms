არ დაგავიწყდეთ, რომ ტერმინალში შეყვანილ ბრძანებებში ნავიგაცია შესაძლებელია up/down arrow key-ებით.


## დავალების ატვირთვა
### ავტომატურად
დააინსტალირეთ პროგრამა zip
გაუშვით ბრძანება `sh zip.sh` დავალების დირექტორიაში. შეიყვანეთ მეილის აიდი (შემდეგი დავალებისთვის დაიმახსოვრებს^^ თუ არასწორად შეიყვანთ, შეცვალეთ ფაილში `~/.config/paradigms`. 

### manual
- დაზიპეთ ფაილი `rsg.cc`. **აუცილებელია იყოს `.zip` და არა rar**. დაზიპეთ **მხოლოდ** ეს ფაილი და არა დირექტორია
- დაარქვით ზიპს თქვენი მეილის id

## setup
0. install valgrind
```sh
sudo apt-get install valgrind #ubuntu
yay -S valgrind #arch
```

change script permissions
```sh
chmod u+x rsgChecker*
chmod u+x rsg-sample*
```

1. build: `make`

2. run: `./rsg data/bionic.g`

3. test
**გაითვალისწინეთ, რომ ტესტერებისთვის სასვენი ნიშნების წინ აუცილებელია იყოს `space`**
```sh
./rsgChecker32 ./rsg data/bionic.g
./rsgChecker64 ./rsg data/bionic.g
```

4. test all
```sh
for i in $(/bin/ls data); do
	echo $i
	./rsgChecker64 ./rsg data/$i
done
```

one line
```sh
for i in $(/bin/ls data); do echo $i; ./rsgChecker64 ./rsg data/$i; done
```
