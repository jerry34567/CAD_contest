# CAD_contest

# setup
if you don't have setuped environment, you have to type command below. (if you have setuped once, ommit the step.)

```json=
cd src/
mkdir eng
cd ..
mkdir include
mkdir lib
sudo ./SETUP.sh 
sudo ./INSTALL.sh
```

# execute
put cir1.v and cir2.v in this directory, then type
```
make
./gv test
```
it will generate 6 files
```
top1.aig
top1.cnf
top2.aig
top2.cnf
map1
map2
```
then you can use aigtoaig to conver .aig to .aag, type
```
./aigtoaig <file_name>.aig <file_name>.aag
``` 