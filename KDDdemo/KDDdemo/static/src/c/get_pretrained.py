from wikipedia2vec import Wikipedia2Vec
import numpy as np
np.random.seed(12345)


def filtered_vectors(id2word, emb_dim):
    count = 0
    wiki2vec = Wikipedia2Vec.load('/shared/data/yumeng5/CatEm/pretrained/enwiki_20180420_100d.pkl')
    #vector = wiki2vec.get_word_vector('from')
    #print(vector)
    w_weight = np.zeros((len(id2word), emb_dim))
    v_weight = np.zeros((len(id2word), emb_dim))
    #print(len(id2word))
    for wid in id2word:
        word = id2word[wid]
        if '_' in word: # it is a phrase
            phrase = word.split('_')
            w_rep = []
            v_rep = []
            for w in phrase:
                try:
                    w_rep.append(wiki2vec.get_word_vector(w))
                    v_rep.append(wiki2vec.syn1[wiki2vec.dictionary.get_word(w).index])
                except KeyError:
                    continue
            if w_rep != []:
                w_weight[wid] = np.average(np.array(w_rep), axis=0)
                v_weight[wid] = np.average(np.array(v_rep), axis=0)
            else:
                # print(id2word[wid])
                count += 1
                w_weight[wid] = np.random.uniform(-0.25, 0.25, 100)
                v_weight[wid] = np.random.uniform(-0.25, 0.25, 100)
        else:
            try:
                w_weight[wid] = wiki2vec.get_word_vector(word)
                v_weight[wid] = wiki2vec.syn1[wiki2vec.dictionary.get_word(word).index]
            except KeyError:
                # print(id2word[wid])
                count += 1
                w_weight[wid] = np.random.uniform(-0.25, 0.25, 100)
                v_weight[wid] = np.random.uniform(-0.25, 0.25, 100)
    #print(id2word[0])
    #print(w_weight[0])
    #print(w_weight[1])
    print(f"Words not appearing in pretrained vocabulary: {np.round(100*count/len(id2word), 3)} %")
    # print(w_weight.shape)
    return w_weight, v_weight


def get_c_pretrain():
    with open('vocabs.txt', 'r') as f:
        words = f.readlines()
        words = [x.strip() for x in words[1:]]
    id2word = {k: v for k, v in enumerate(words)}
    w, v = filtered_vectors(id2word, 100)

    for name, embs in zip(['emb_w.txt', 'emb_v.txt'], [w, v]):
        with open(name, 'w') as f:
            for emb in embs:
                emb = ' '.join(list(map(str, emb)))
                f.write(f'{emb}\n')


if __name__ == '__main__':
    get_c_pretrain()
