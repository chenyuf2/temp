# dataset directory
dataset=../papers

# text file name; one document per line
text_file=text.txt

topic_file=dm

if [ ! -e ./${dataset}/emb_${topic_file}_gjhghg.txt ]
then
		echo ${green}===Step 2: Word Embedding Training===${reset}
		./word2vec -train ./${dataset}/${text_file} -output ./${dataset}/emb_${topic_file}_w.txt -kappa ./${dataset}/emb_${topic_file}_cap.txt -topic ./${dataset}/topics_${topic_file}.txt -topic_output ./${dataset}/emb_${topic_file}_t.txt -reg_lambda 10 -cbow 0 -size 100 -global_lambda 1.5 -window 5 -negative 5 -sample 1e-3 -min-count 5 -threads 20 -binary 0 -iter 2  -pretrain 1 -rank_product 0 -gen_vocab 0 -load_emb 1
		echo ${green}===Step 3: Word Embedding Training Skipped\; Using Trained Embedding to Generate Keywords===${reset}
    python ../eval_c.py --dataset ${dataset} --topic_file topics_${topic_file}.txt --emb emb_${topic_file} --out_file results_${topic_file}.txt
    python ../eval_copy.py --dataset ${dataset} --topic_file topics_${topic_file}.txt --emb emb_${topic_file} --out_file results2_${topic_file}.txt
else
    echo ${green}===Step 3: Word Embedding Training Skipped\; Using Trained Embedding to Generate Keywords===${reset}
    python ../eval_c.py --dataset ${dataset} --topic_file topics_${topic_file}.txt --emb emb_${topic_file} --out_file results_${topic_file}.txt
fi
