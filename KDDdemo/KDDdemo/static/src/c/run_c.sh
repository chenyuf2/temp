# dataset directory
dataset=../nyt_ann

# text file name; one document per line
text_file=text_new.txt

green=`tput setaf 2`
reset=`tput sgr0`

if [ ! -e ./${dataset}/cleaned.txt ]
then
    echo ${green}===Step 1: Pre-processing===${reset}
    python preprocess.py --dataset ${dataset} --in_file ${text_file} --out_file cleaned.txt
else
    echo ${green}===Step 1: Pre-processing Skipped\; Using Pre-processed File===${reset}
fi

if [ ! -e ./${dataset}/embedding_level1noproduct.txt ]
then
    echo ${green}===Step 2: Word Embedding Training===${reset}
    ### generate vocabulary and pretrained embedding
    # ./word2vec -train ./${dataset}/cleaned.txt -output ./${dataset}/embedding_c_w.txt -kappa ./${dataset}/embedding_c_cap.txt -topic ./${dataset}/topics.txt -topic_output ./${dataset}/embedding_c_t.txt -reg_lambda 1 -cbow 0 -size 100 -global_lambda 1.5 -window 5 -negative 5 -sample 1e-3 -min-count 5 -threads 20 -binary 0 -iter 10 -pretrain 2 -rank_product 0 -gen_vocab 1
    # python get_pretrained.py

    ## load pretrained embedding
    ./word2vec -train ./${dataset}/cleaned.txt -output ./${dataset}/embedding_c_w.txt -context ./${dataset}/embedding_c_v.txt -kappa ./${dataset}/embedding_c_cap.txt -topic ./${dataset}/verbs.txt -topic_output ./${dataset}/embedding_c_t.txt -reg_lambda 1 -cbow 0 -size 100 -with_local 1 -global_lambda 1.5 -window 5 -negative 5 -sample 1e-3 -min-count 5 -threads 20 -binary 0 -iter 10 -pretrain 2 -rank_product 1 -gen_vocab 0 -load_emb 1

    ## load embedding previously trained by this model
    # ./word2vec -train ./${dataset}/cleaned.txt -output ./${dataset}/embedding_c_w.txt -context ./${dataset}/embedding_c_v.txt -kappa ./${dataset}/embedding_c_cap.txt -topic ./${dataset}/verb.txt -topic_output ./${dataset}/embedding_c_t.txt -reg_lambda 1 -cbow 0 -size 100 -with_local 1 -global_lambda 1.5 -window 5 -negative 5 -sample 1e-3 -min-count 5 -threads 20 -binary 0 -iter 10 -pretrain 2 -rank_product 1 -gen_vocab 0 -load_emb_with_v 1 -load_emb_file ./${dataset}/embedding_c
else
    echo ${green}===Step 2: Word Embedding Training Skipped\; Using Trained Embedding===${reset}
fi
