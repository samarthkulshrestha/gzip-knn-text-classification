# text-classification with gzip

text-classification using gzip normalised compression distances (NCD) and
k-nearest-neighbours (KNN).

### building

```console
cc -o nob nob.c
./nob
```

### usage

```console
./main data/train.csv data/test.csv
```

you will see the result:

```
classifying 119999/120000
[INFO] text: Sinner rallies from 2 sets down to beat Medvedev in Australia and clinch his first Grand Slam title. Jannik Sinner has rallied from two sets down to win the Australian Open final against Daniil Medvedev and clinch his first Grand Slam title.
[INFO] class: Sports
```

the training and test data resides in `data/train.csv` and `data/test.csv`
respectively, while the text that needs to be classified is set in the `text`
variable in the `main()` function of `src/main.c`.

### contribute

+ i <3 pull requests and bug reports!
+ don't hesitate to [tell me my code-fu sucks](https://github.com/samarthkulshrestha/lego/issues/new), but please tell me why.
+ feel free to fork the project and try out your own optimisations.

### license

gzip-knn-text-classification is licensed under the MIT License.

Copyright (c) 2024 Samarth Kulshrestha.
