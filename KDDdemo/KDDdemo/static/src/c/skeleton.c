//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define MAX_STRING 100
#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6
#define MAX_SENTENCE_LENGTH 1000
#define MAX_CODE_LENGTH 40

// windows mingw
//#define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ?0 :errno)


const int vocab_hash_size = 30000000;  // Maximum 30 * 0.7 = 21M words in the vocabulary
const int corpus_max_size = 30000000;  // Maximum 30M documents in the corpus

typedef float real;                    // Precision of float numbers

struct vocab_word {
    long long cn;
    int *point;
    char *word, *code, codelen;
};

char train_file[MAX_STRING], output_file[MAX_STRING];
char save_vocab_file[MAX_STRING], read_vocab_file[MAX_STRING];
struct vocab_word *vocab;
int binary = 0, cbow = 0, debug_mode = 2, window = 5, min_count = 5, num_threads = 12, min_reduce = 1;
int *vocab_hash, *docs;
long long *doc_sizes;
long long vocab_max_size = 1000, vocab_size = 0, corpus_size = 0, layer1_size = 100;
long long train_words = 0, word_count_actual = 0, iter = 10, file_size = 0, classes = 0;
real alpha = 0.025, starting_alpha, lambda = 1.5, sample = 1e-3;
real *syn0, *syn1, *syn1neg, *syn1doc, *expTable;
real *kappa;
clock_t start;

int global = 1;
int regularization = 1;

int negative = 5;
const int table_size = 1e8;
int *word_table, *doc_table;

// regularization:
int topics = 10; // number of topics, t_embedding.size(0)
real *t_embeddings;
int expand = 1; // iter * expand + 1 == rankings.size()
int num_per_topic;
real *wt_score; // similarity between topic and word
int *rankings; // ranking for each topic
real lambda_3 = 1.0; // scaling for regularization

int rankingComparator(const void *a, const void *b) {
  return (wt_score[*(int *) a] < wt_score[*(int *) b]) - (wt_score[*(int *) a] > wt_score[*(int *) b]);
}

void InitUnigramTable() {
  int a, i;
  double train_words_pow = 0;
  double d1, power = 0.75;
  word_table = (int *) malloc(table_size * sizeof(int));
  for (a = 0; a < vocab_size; a++) train_words_pow += pow(vocab[a].cn, power);
  i = 0;
  d1 = pow(vocab[i].cn, power) / train_words_pow;
  for (a = 0; a < table_size; a++) {
    word_table[a] = i;
    if (a / (double) table_size > d1) {
      i++;
      d1 += pow(vocab[i].cn, power) / train_words_pow;
    }
    if (i >= vocab_size) i = vocab_size - 1;
  }
}

void InitDocTable() {
  int a, i;
  double doc_len_pow = 0;
  double d1, power = 0.75;
  doc_table = (int *) malloc(table_size * sizeof(int));
  for (a = 0; a < corpus_size; a++) doc_len_pow += pow(docs[a], power);
  i = 0;
  d1 = pow(docs[i], power) / doc_len_pow;
  for (a = 0; a < table_size; a++) {
    doc_table[a] = i;
    if (a / (double) table_size > d1) {
      i++;
      d1 += pow(docs[i], power) / doc_len_pow;
    }
    if (i >= corpus_size) i = corpus_size - 1;
  }
}

// Reads a single word from a file, assuming space + tab + EOL to be word boundaries
void ReadWord(char *word, FILE *fin) {
  int a = 0, ch;
  while (!feof(fin)) {
    ch = fgetc(fin);
    if (ch == 13) continue;
    if ((ch == ' ') || (ch == '\t') || (ch == '\n')) {
      if (a > 0) {
        if (ch == '\n') ungetc(ch, fin);
        break;
      }
      if (ch == '\n') {
        strcpy(word, (char *) "</s>");
        return;
      } else continue;
    }
    word[a] = ch;
    a++;
    if (a >= MAX_STRING - 1) a--;   // Truncate too long words
  }
  word[a] = 0;
}

// Returns hash value of a word
int GetWordHash(char *word) {
  unsigned long long a, hash = 0;
  for (a = 0; a < strlen(word); a++) hash = hash * 257 + word[a];
  hash = hash % vocab_hash_size;
  return hash;
}

// Returns position of a word in the vocabulary; if the word is not found, returns -1
int SearchVocab(char *word) {
  unsigned int hash = GetWordHash(word);
  while (1) {
    if (vocab_hash[hash] == -1) return -1;
    if (!strcmp(word, vocab[vocab_hash[hash]].word)) return vocab_hash[hash];
    hash = (hash + 1) % vocab_hash_size;
  }
  return -1;
}

// Record document length
void ReadDoc(FILE *fin) {
  char word[MAX_STRING];
  long long i;
  while (1) {
    ReadWord(word, fin);
    if (feof(fin)) break;
    i = SearchVocab(word);
    if (i == 0) {
      doc_sizes[corpus_size] = ftell(fin);
      corpus_size++;
    } else if (i == -1) continue;
    else docs[corpus_size]++;
  }
  // for (i = 0; i <= 5; i++) printf("%lld\n", doc_sizes[i]);
}

// Locate line number of current file pointer
int FindLine(FILE *fin) {
  int i;
  for (i = 0; i < corpus_size; i++) {
    if (doc_sizes[i] > ftell(fin)) break;
  }
  return i;
}

// Reads a word and returns its index in the vocabulary
int ReadWordIndex(FILE *fin) {
  char word[MAX_STRING];
  ReadWord(word, fin);
  if (feof(fin)) return -1;
  return SearchVocab(word);
}

// Adds a word to the vocabulary
int AddWordToVocab(char *word) {
  unsigned int hash, length = strlen(word) + 1;
  if (length > MAX_STRING) length = MAX_STRING;
  vocab[vocab_size].word = (char *) calloc(length, sizeof(char));
  strcpy(vocab[vocab_size].word, word);
  vocab[vocab_size].cn = 0;
  vocab_size++;
  // Reallocate memory if needed
  if (vocab_size + 2 >= vocab_max_size) {
    vocab_max_size += 1000;
    vocab = (struct vocab_word *) realloc(vocab, vocab_max_size * sizeof(struct vocab_word));
  }
  hash = GetWordHash(word);
  while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
  vocab_hash[hash] = vocab_size - 1;
  return vocab_size - 1;
}

// Used later for sorting by word counts
int VocabCompare(const void *a, const void *b) {
  return ((struct vocab_word *) b)->cn - ((struct vocab_word *) a)->cn;
}

// Sorts the vocabulary by frequency using word counts
void SortVocab() {
  int a, size;
  unsigned int hash;
  // Sort the vocabulary and keep </s> at the first position
  qsort(&vocab[1], vocab_size - 1, sizeof(struct vocab_word), VocabCompare);
  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
  size = vocab_size;
  train_words = 0;
  for (a = 0; a < size; a++) {
    // Words occuring less than min_count times will be discarded from the vocab
    if ((vocab[a].cn < min_count) && (a != 0)) {
      vocab_size--;
      free(vocab[a].word);
    } else {
      // Hash will be re-computed, as after the sorting it is not actual
      hash = GetWordHash(vocab[a].word);
      while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
      vocab_hash[hash] = a;
      train_words += vocab[a].cn;
    }
  }
  vocab = (struct vocab_word *) realloc(vocab, (vocab_size + 1) * sizeof(struct vocab_word));
  // Allocate memory for the binary tree construction
  for (a = 0; a < vocab_size; a++) {
    vocab[a].code = (char *) calloc(MAX_CODE_LENGTH, sizeof(char));
    vocab[a].point = (int *) calloc(MAX_CODE_LENGTH, sizeof(int));
  }
}

// Reduces the vocabulary by removing infrequent tokens
void ReduceVocab() {
  int a, b = 0;
  unsigned int hash;
  for (a = 0; a < vocab_size; a++)
    if (vocab[a].cn > min_reduce) {
      vocab[b].cn = vocab[a].cn;
      vocab[b].word = vocab[a].word;
      b++;
    } else free(vocab[a].word);
  vocab_size = b;
  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
  for (a = 0; a < vocab_size; a++) {
    // Hash will be re-computed, as it is not actual
    hash = GetWordHash(vocab[a].word);
    while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
    vocab_hash[hash] = a;
  }
  fflush(stdout);
  min_reduce++;
}

void LearnVocabFromTrainFile() {
  char word[MAX_STRING];
  FILE *fin;
  long long a, i;
  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
  fin = fopen(train_file, "rb");
  if (fin == NULL) {
    printf("ERROR: training data file not found!\n");
    exit(1);
  }
  vocab_size = 0;
  AddWordToVocab((char *) "</s>");
  while (1) {
    ReadWord(word, fin);
    if (feof(fin)) break;
    train_words++;
    if ((debug_mode > 1) && (train_words % 100000 == 0)) {
      printf("%lldK%c", train_words / 1000, 13);
      fflush(stdout);
    }
    i = SearchVocab(word);
    if (i == -1) {
      a = AddWordToVocab(word);
      vocab[a].cn = 1;
    } else vocab[i].cn++;
    if (vocab_size > vocab_hash_size * 0.7) ReduceVocab();
  }
  SortVocab();
  if (debug_mode > 0) {
    printf("Vocab size: %lld\n", vocab_size);
    printf("Words in train file: %lld\n", train_words);
  }
  fseek(fin, 0, SEEK_SET);
  ReadDoc(fin);
  file_size = ftell(fin);
  fclose(fin);
}

void SaveVocab() {
  long long i;
  FILE *fo = fopen(save_vocab_file, "wb");
  for (i = 0; i < vocab_size; i++) fprintf(fo, "%s %lld\n", vocab[i].word, vocab[i].cn);
  fclose(fo);
}

void ReadVocab() {
  long long a, i = 0;
  char c;
  char word[MAX_STRING];
  FILE *fin = fopen(read_vocab_file, "rb");
  if (fin == NULL) {
    printf("Vocabulary file not found\n");
    exit(1);
  }
  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
  vocab_size = 0;
  while (1) {
    ReadWord(word, fin);
    if (feof(fin)) break;
    a = AddWordToVocab(word);
    fscanf(fin, "%lld%c", &vocab[a].cn, &c);
    i++;
  }
  SortVocab();
  if (debug_mode > 0) {
    printf("Vocab size: %lld\n", vocab_size);
    printf("Words in train file: %lld\n", train_words);
  }
  fin = fopen(train_file, "rb");
  if (fin == NULL) {
    printf("ERROR: training data file not found!\n");
    exit(1);
  }
  fseek(fin, 0, SEEK_END);
  file_size = ftell(fin);
  fclose(fin);
}

void InitNet() {
  long long a, b;
  unsigned long long next_random = 1;
  a = posix_memalign((void **) &syn0, 128, (long long) vocab_size * layer1_size * sizeof(real));
  if (syn0 == NULL) {
    printf("Memory allocation failed\n");
    exit(1);
  }
  a = posix_memalign((void **) &kappa, 128, (long long) vocab_size * sizeof(real));
  for (a = 0; a < vocab_size; a++) kappa[a] = 1.0;
  if (negative > 0) {
    a = posix_memalign((void **) &syn1neg, 128, (long long) vocab_size * layer1_size * sizeof(real));
    a = posix_memalign((void **) &syn1doc, 128, (long long) corpus_size * layer1_size * sizeof(real));
    if (syn1neg == NULL) {
      printf("Memory allocation failed (syn1neg)\n");
      exit(1);
    }
    if (syn1doc == NULL) {
      printf("Memory allocation failed (syn1doc)\n");
      exit(1);
    }
    for (a = 0; a < vocab_size; a++)
      for (b = 0; b < layer1_size; b++)
        syn1neg[a * layer1_size + b] = 0;
    for (a = 0; a < corpus_size; a++)
      for (b = 0; b < layer1_size; b++)
        syn1doc[a * layer1_size + b] = 0;
  }
  for (a = 0; a < vocab_size; a++)
    for (b = 0; b < layer1_size; b++) {
      next_random = next_random * (unsigned long long) 25214903917 + 11;
      syn0[a * layer1_size + b] = (((next_random & 0xFFFF) / (real) 65536) - 0.5) / layer1_size;
    }
  // regularization
  FILE *fp = fopen("locations.txt", "r");
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int iidx = 0;
  int *topic_index = calloc(topics, sizeof(int));
  while ((read = getline(&line, &len, fp)) != -1) {
//    printf("Retrieved line of length %zu:\n", read);
//    printf("%s\n", line);
//    printf("%zu\n", strlen(line));
    line[read - 2] = 0;
//    printf("%zu %s\n", read, line);
//    printf("%zu\n", strlen(line));
//    if (strcmp("united_states", line) != 0) {
//      printf("%s\n AA", line);
//    }
    topic_index[iidx++] = SearchVocab(line);
  }
  fclose(fp);
  for (a = 0; a < topics; a++) {
    printf("%d %s\n", topic_index[a], vocab[topic_index[a]].word);
  }
  a = posix_memalign((void **) &t_embeddings, 128, (long long) topics * layer1_size * sizeof(real));
  for (a = 0; a < topics; a++)
    for (b = 0; b < layer1_size; b++) {
      t_embeddings[a * layer1_size + b] = syn0[topic_index[a] * layer1_size + b];
//      next_random = next_random * (unsigned long long) 25214903917 + 11;
//      t_embeddings[a * layer1_size + b] = (((next_random & 0xFFFF) / (real) 65536) - 0.5) / layer1_size;
    }
  a = posix_memalign((void **) &wt_score, 128, (long long) topics * vocab_size * sizeof(real));
  a = posix_memalign((void **) &rankings, 128, (long long) topics * vocab_size * sizeof(int));
  for (a = 0; a < topics * vocab_size; a++) rankings[a] = a;
}

void *TrainModelThread(void *id) {
  long long a, b, d, doc, cw, word, last_word, sentence_length = 0, sentence_position = 0;
  long long word_count = 0, last_word_count = 0, sen[MAX_SENTENCE_LENGTH + 1];
  long long l1, l2, c, target, label, local_iter = 1;
  unsigned long long next_random = (long long) id;
  real f, g;
  clock_t now;
  real *neu1 = (real *) calloc(layer1_size, sizeof(real));
  real *neu1e = (real *) calloc(layer1_size, sizeof(real));
  FILE *fi = fopen(train_file, "rb");
  fseek(fi, file_size / (long long) num_threads * (long long) id, SEEK_SET);

  // regularization
  real *wi = (real *) calloc(topics * num_per_topic, sizeof(real));
  real *exp_rij = (real *) calloc(topics * num_per_topic * topics, sizeof(real));
  real *delta_tjk = (real *) calloc(topics * layer1_size, sizeof(real));
  real *delta_wik = (real *) calloc(topics * num_per_topic * layer1_size, sizeof(real));
  int words_per_reg = 200;
  int word_counter = 0;
  while (1) {
//      printf("%lld %lld\n", word_count, last_word_count);
    if (word_count - last_word_count > 10000) {
      word_count_actual += word_count - last_word_count;
      last_word_count = word_count;
      if ((debug_mode > 1)) {
        now = clock();
        printf("%cAlpha: %f  Progress: %.2f%%  Words/thread/sec: %.2fk  ", 13, alpha,
               word_count_actual / (real) (iter * train_words + 1) * 100,
               word_count_actual / ((real) (now - start + 1) / (real) CLOCKS_PER_SEC * 1000));
        fflush(stdout);
      }
      alpha = starting_alpha * (1 - word_count_actual / (real) (iter * train_words + 1));
      if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
    }

    if (sentence_length == 0) {
      doc = FindLine(fi);
      while (1) {
        word = ReadWordIndex(fi);
        if (feof(fi)) break;
        if (word == -1) continue;
        word_count++;
        if (word == 0) break;
        // The subsampling randomly discards frequent words while keeping the ranking same
        if (sample > 0) {
          real ran = (sqrt(vocab[word].cn / (sample * train_words)) + 1) * (sample * train_words) /
                     vocab[word].cn;
          next_random = next_random * (unsigned long long) 25214903917 + 11;
          if (ran < (next_random & 0xFFFF) / (real) 65536) continue;
        }
        sen[sentence_length] = word;
        sentence_length++;
        if (sentence_length >= MAX_SENTENCE_LENGTH) break;
      }
      sentence_position = 0;
    }

    if (feof(fi) || (word_count > train_words / num_threads)) {
      word_count_actual += word_count - last_word_count;
      local_iter--;
      if (local_iter == 0) break;
      word_count = 0;
      last_word_count = 0;
      sentence_length = 0;
      fseek(fi, file_size / (long long) num_threads * (long long) id, SEEK_SET);
      continue;
    }

    // regularization
    if (regularization) {
      word_counter += 1;
      if (word_counter % words_per_reg == 0) {
        real L = 0.0;
        for (a = 0; a < topics; a++)
          for (b = 0; b < num_per_topic; b++) {
            int word_index = rankings[a * vocab_size + b] % vocab_size;
            wi[a * num_per_topic + b] = 0;
            real rci = 0.0;

            for (d = 0; d < topics; d++) {
              real rij = 0.0;
              for (c = 0; c < layer1_size; c++) {
                rij += syn0[word_index * layer1_size + c] * t_embeddings[d * layer1_size + c];
              }
              real eij = exp(rij);

              exp_rij[(a * num_per_topic + b) * topics + d] = eij;
              wi[a * num_per_topic + b] += eij;
              if (a == d) rci = rij;
            }
            L += -rci + log(wi[a * num_per_topic + b]);
            for (c = 0; c < layer1_size; c++) {
              int wik_index = (a * num_per_topic + b) * layer1_size + c;
              delta_wik[wik_index] = 0.0;
              for (d = 0; d < topics; d++) {
                delta_wik[wik_index] += t_embeddings[d * layer1_size + c] *
                                        (exp_rij[(a * num_per_topic + b) * topics + d] /
                                         wi[a * num_per_topic + b] - (a == d ? 1 : 0));
              }
            }
          }

        for (d = 0; d < topics; d++)
          for (c = 0; c < layer1_size; c++) {
            int tjk_index = d * layer1_size + c;
            delta_tjk[tjk_index] = 0.0;
            for (a = 0; a < topics; a++)
              for (b = 0; b < num_per_topic; b++) {
                int word_index = rankings[a * vocab_size + b] % vocab_size;
                delta_tjk[tjk_index] += syn0[word_index * layer1_size + c] *
                                        (exp_rij[(a * num_per_topic + b) * topics + d] /
                                         wi[a * num_per_topic + b] - (a == d ? 1 : 0));
              }
          }
      }
    }


    word = sen[sentence_position];
    // printf("(%lld, %s)", doc, vocab[word].word);
    if (word == -1) continue;
    for (c = 0; c < layer1_size; c++) neu1[c] = 0;
    for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
    next_random = next_random * (unsigned long long) 25214903917 + 11;
    // b = next_random % window;
    b = 0;
    if (cbow) {  //train the cbow architecture
//      // in -> hidden
//      cw = 0;
//      for (a = b; a < window * 2 + 1 - b; a++)
//        if (a != window) {
//          c = sentence_position - window + a;
//          if (c < 0) continue;
//          if (c >= sentence_length) continue;
//          last_word = sen[c];
//          if (last_word == -1) continue;
//          for (c = 0; c < layer1_size; c++) neu1[c] += syn0[c + last_word * layer1_size];
//          cw++;
//        }
//      if (cw) {
//        for (c = 0; c < layer1_size; c++) neu1[c] /= cw;
//        // NEGATIVE SAMPLING
//        if (negative > 0)
//          for (d = 0; d < negative + 1; d++) {
//            if (d == 0) {
//              target = word;
//              label = 1;
//            } else {
//              next_random = next_random * (unsigned long long) 25214903917 + 11;
//              target = word_table[(next_random >> 16) % table_size];
//              if (target == 0) target = next_random % (vocab_size - 1) + 1;
//              if (target == word) continue;
//              label = 0;
//            }
//            l2 = target * layer1_size;
//            f = 0;
//            for (c = 0; c < layer1_size; c++) f += neu1[c] * syn1neg[c + l2];
//            if (f > MAX_EXP) g = (label - 1) * alpha;
//            else if (f < -MAX_EXP) g = (label - 0) * alpha;
//            else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
//            // printf("word gradient: %.5f\n", g);
//            // g *= 1/(1 + lambda);
//            for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
//            for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * neu1[c];
//          }
//        // hidden -> in
//        for (a = b; a < window * 2 + 1 - b; a++)
//          if (a != window) {
//            c = sentence_position - window + a;
//            if (c < 0) continue;
//            if (c >= sentence_length) continue;
//            last_word = sen[c];
//            if (last_word == -1) continue;
//            for (c = 0; c < layer1_size; c++) syn0[c + last_word * layer1_size] += neu1e[c];
//          }
//      }
//      if (negative > 0)
//        for (d = 0; d < negative + 1; d++) {
//          if (d == 0) {
//            target = word;
//            label = 1;
//          } else {
//            next_random = next_random * (unsigned long long) 25214903917 + 11;
//            target = word_table[(next_random >> 16) % table_size];
//            if (target == 0) target = next_random % (vocab_size - 1) + 1;
//            if (target == word) continue;
//            label = 0;
//          }
//          l2 = target * layer1_size;
//          f = 0;
//          for (c = 0; c < layer1_size; c++) f += syn1doc[c + doc * layer1_size] * syn1neg[c + l2];
//          if (f > MAX_EXP) g = (label - 1) * alpha;
//          else if (f < -MAX_EXP) g = (label - 0) * alpha;
//          else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
//          // g *= lambda/(1 + lambda);
//          g *= lambda;
//          // printf("doc gradient: %.5f\n", g);
//          // printf("progress: %.5f%%\n", word_count_actual / (real)(iter * train_words + 1) * 100);
//          for (c = 0; c < layer1_size; c++) syn1doc[c + doc * layer1_size] += g * syn1neg[c + l2];
//          for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * syn1doc[c + doc * layer1_size];
//        }

    } else {  //train skip-gram
      for (a = b; a < window * 2 + 1 - b; a++)
        if (a != window) {
          c = sentence_position - window + a;
          if (c < 0) continue;
          if (c >= sentence_length) continue;


          last_word = sen[c];
          if (last_word == -1) continue;
          l1 = last_word * layer1_size;
          for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
          // NEGATIVE SAMPLING
          if (negative > 0) {
            real kappa_update = 0.0;
            for (d = 0; d < negative + 1; d++) {
              if (d == 0) {
                target = word;
                label = 1;
              } else {
                next_random = next_random * (unsigned long long) 25214903917 + 11;
                target = word_table[(next_random >> 16) % table_size];
                if (target == 0) target = next_random % (vocab_size - 1) + 1;
                if (target == word) continue;
                label = 0;
              }
              l2 = target * layer1_size;
              f = 0;
              for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1neg[c + l2];
              real tmp_kappa_update = f;
              f *= kappa[last_word];
              if (f > MAX_EXP) g = (label - 1) * alpha;
              else if (f < -MAX_EXP) g = (label - 0) * alpha;
              else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
              kappa_update += g * tmp_kappa_update;
              // g *= 1/(1 + lambda);
              for (c = 0; c < layer1_size; c++) neu1e[c] += g * kappa[last_word] * syn1neg[c + l2];
              for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * kappa[last_word] * syn0[c + l1];
            }
            // Learn weights input -> hidden
            for (c = 0; c < layer1_size; c++) syn0[c + l1] += neu1e[c];
            kappa[last_word] += kappa_update;
          }
        }
      if (global) {
        for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
        if (negative > 0) {
          real kappa_update = 0.0;
          for (d = 0; d < negative + 1; d++) {
            if (d == 0) {
              target = doc;
              label = 1;
            } else {
              next_random = next_random * (unsigned long long) 25214903917 + 11;
              target = doc_table[(next_random >> 16) % table_size];
              if (target == doc) continue;
              label = 0;
            }
            l2 = target * layer1_size;
            f = 0;
            for (c = 0; c < layer1_size; c++) {
              f += syn0[c + word * layer1_size] * syn1doc[c + l2];
            }
            real tmp_kappa_update = f;
            f *= kappa[word];
            if (f > MAX_EXP) g = (label - 1) * alpha;
            else if (f < -MAX_EXP) g = (label - 0) * alpha;
            else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
            // g *= lambda/(1 + lambda);
            g *= lambda;
            kappa_update += g * tmp_kappa_update;
            for (c = 0; c < layer1_size; c++) neu1e[c] += g * kappa[word] * syn1doc[c + l2];
            for (c = 0; c < layer1_size; c++) syn1doc[c + l2] += g * kappa[word] * syn0[c + word * layer1_size];
          }
          for (c = 0; c < layer1_size; c++) syn0[c + word * layer1_size] += neu1e[c];
          kappa[word] += kappa_update;
        }
      }
    }

    // regularization
    if (regularization) {
      if (word_counter % words_per_reg == 0) {
        real scaling = -alpha * lambda_3 / (topics * num_per_topic);
        for (a = 0; a < topics; a++)
          for (b = 0; b < num_per_topic; b++) {
            int word_index = rankings[a * vocab_size + b] % vocab_size;
            for (c = 0; c < layer1_size; c++) {
              syn0[word_index * layer1_size + c] +=
                  scaling * delta_wik[(a * num_per_topic + b) * layer1_size + c];
            }
          }
        for (d = 0; d < topics; d++)
          for (c = 0; c < layer1_size; c++) {
            t_embeddings[d * layer1_size + c] += scaling * delta_tjk[d * layer1_size + c];
          }
//      lambda_3 -= alpha * L / (topics * num_per_topic);
      }
    }

    sentence_position++;
    if (sentence_position >= sentence_length) {
      sentence_length = 0;
      continue;
    }
  }
  fclose(fi);
  free(neu1);
  free(neu1e);
  pthread_exit(NULL);
}

void TrainModel() {
  long a, b, c, d;
  long iter_count;
  FILE *fo;
  pthread_t *pt = (pthread_t *) malloc(num_threads * sizeof(pthread_t));
  printf("Starting training using file %s\n", train_file);
  starting_alpha = alpha;
  if (read_vocab_file[0] != 0) ReadVocab(); else LearnVocabFromTrainFile();
  if (save_vocab_file[0] != 0) SaveVocab();
  if (output_file[0] == 0) return;
  // exit(0);
  InitNet();
  if (negative > 0) {
    InitUnigramTable();
    InitDocTable();
  }
  start = clock();
  for (iter_count = 0; iter_count < iter; iter_count++) {
    num_per_topic = iter_count * expand + 1;
    for (a = 0; a < topics; a++)
      for (b = 0; b < vocab_size; b++) {
        wt_score[a * vocab_size + b] = 0;
        real norm = 0.0;
        for (c = 0; c < layer1_size; c++) {
          wt_score[a * vocab_size + b] += t_embeddings[a * layer1_size + c] * syn0[b * layer1_size + c];
          norm += syn0[b * layer1_size + c] * syn0[b * layer1_size + c];
        }
        wt_score[a * vocab_size + b] /= sqrt(norm);
      }
    // skip normalization.
    // can use select instead of sort
    for (a = 0; a < topics; a++) qsort(rankings + a * vocab_size, vocab_size, sizeof(int), rankingComparator);
    for (a = 0; a < topics; a++) {
      printf("Cluster: %ld\n", a);
      for (b = 0; b < num_per_topic; b++) {
        printf("%s ", vocab[rankings[a * vocab_size + b] % vocab_size].word);
      }
      printf("\n");
    }
    for (a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, TrainModelThread, (void *) a);
    for (a = 0; a < num_threads; a++) pthread_join(pt[a], NULL);
  }
  fo = fopen(output_file, "wb");
  FILE *fp = fopen(kappa_file, "wb");
  if (classes == 0) {
    // Save the word vectors
    fprintf(fo, "%lld %lld\n", vocab_size, layer1_size);
    for (a = 0; a < vocab_size; a++) {
      fprintf(fo, "%s ", vocab[a].word);
      if (binary)
        for (b = 0; b < layer1_size; b++) {
          if (cbow) fwrite(&syn1neg[a * layer1_size + b], sizeof(real), 1, fo);
          else fwrite(&syn0[a * layer1_size + b], sizeof(real), 1, fo);
        }
      else
        for (b = 0; b < layer1_size; b++) {
          if (cbow) fprintf(fo, "%lf ", syn1neg[a * layer1_size + b]);
          else fprintf(fo, "%lf ", syn0[a * layer1_size + b]);
        }
      fprintf(fo, "\n");
    }
    fprintf(fp, "%lld\n", vocab_size);
    for (a = 0; a < vocab_size; a++) {
      fprintf(fp, "%s ", vocab[a].word);
//      real norm = 0.0;
//      for (b = 0; b < layer1_size; b++) {
//        norm += syn0[a * layer1_size + b] * syn0[a * layer1_size + b];
//      }
      fprintf(fp, "%lf\n", kappa[a]);
    }
  } else {
    // Run K-means on the word vectors
    int clcn = classes, iter = 10, closeid;
    int *centcn = (int *) malloc(classes * sizeof(int));
    int *cl = (int *) calloc(vocab_size, sizeof(int));
    real closev, x;
    real *cent = (real *) calloc(classes * layer1_size, sizeof(real));
    for (a = 0; a < vocab_size; a++) cl[a] = a % clcn;
    for (a = 0; a < iter; a++) {
      for (b = 0; b < clcn * layer1_size; b++) cent[b] = 0;
      for (b = 0; b < clcn; b++) centcn[b] = 1;
      for (c = 0; c < vocab_size; c++) {
        for (d = 0; d < layer1_size; d++) cent[layer1_size * cl[c] + d] += syn0[c * layer1_size + d];
        centcn[cl[c]]++;
      }
      for (b = 0; b < clcn; b++) {
        closev = 0;
        for (c = 0; c < layer1_size; c++) {
          cent[layer1_size * b + c] /= centcn[b];
          closev += cent[layer1_size * b + c] * cent[layer1_size * b + c];
        }
        closev = sqrt(closev);
        for (c = 0; c < layer1_size; c++) cent[layer1_size * b + c] /= closev;
      }
      for (c = 0; c < vocab_size; c++) {
        closev = -10;
        closeid = 0;
        for (d = 0; d < clcn; d++) {
          x = 0;
          for (b = 0; b < layer1_size; b++) x += cent[layer1_size * d + b] * syn0[c * layer1_size + b];
          if (x > closev) {
            closev = x;
            closeid = d;
          }
        }
        cl[c] = closeid;
      }
    }
    // Save the K-means classes
    for (a = 0; a < vocab_size; a++) fprintf(fo, "%s %d\n", vocab[a].word, cl[a]);
    free(centcn);
    free(cent);
    free(cl);
  }
  fclose(fo);
}

int ArgPos(char *str, int argc, char **argv) {
  int a;
  for (a = 1; a < argc; a++)
    if (!strcmp(str, argv[a])) {
      if (a == argc - 1) {
        printf("Argument missing for %s\n", str);
        exit(1);
      }
      return a;
    }
  return -1;
}

int main(int argc, char **argv) {
  int i;
  if (argc == 1) {
    printf("WORD VECTOR estimation toolkit v 0.1c\n\n");
    printf("Options:\n");
    printf("Parameters for training:\n");
    printf("\t-train <file>\n");
    printf("\t\tUse text data from <file> to train the model\n");
    printf("\t-output <file>\n");
    printf("\t\tUse <file> to save the resulting word vectors / word clusters\n");
    printf("\t-size <int>\n");
    printf("\t\tSet size of word vectors; default is 100\n");
    printf("\t-window <int>\n");
    printf("\t\tSet max skip length between words; default is 5\n");
    printf("\t-sample <float>\n");
    printf("\t\tSet threshold for occurrence of words. Those that appear with higher frequency in the training data\n");
    printf("\t\twill be randomly down-sampled; default is 1e-3, useful range is (0, 1e-5)\n");
    printf("\t-negative <int>\n");
    printf("\t\tNumber of negative examples; default is 5, common values are 3 - 10 (0 = not used)\n");
    printf("\t-threads <int>\n");
    printf("\t\tUse <int> threads (default 12)\n");
    printf("\t-iter <int>\n");
    printf("\t\tRun more training iterations (default 5)\n");
    printf("\t-min-count <int>\n");
    printf("\t\tThis will discard words that appear less than <int> times; default is 5\n");
    printf("\t-alpha <float>\n");
    printf("\t\tSet the starting learning rate; default is 0.025 for skip-gram and 0.05 for CBOW\n");
    printf("\t-lambda <float>\n");
    printf("\t\tSet the relative importance of global context to local context; default is 0.5\n");
    printf("\t-classes <int>\n");
    printf("\t\tOutput word classes rather than word vectors; default number of classes is 0 (vectors are written)\n");
    printf("\t-debug <int>\n");
    printf("\t\tSet the debug mode (default = 2 = more info during training)\n");
    printf("\t-binary <int>\n");
    printf("\t\tSave the resulting vectors in binary moded; default is 0 (off)\n");
    printf("\t-save-vocab <file>\n");
    printf("\t\tThe vocabulary will be saved to <file>\n");
    printf("\t-read-vocab <file>\n");
    printf("\t\tThe vocabulary will be read from <file>, not constructed from the training data\n");
    printf("\t-cbow <int>\n");
    printf("\t\tUse the continuous bag of words model; default is 1 (use 0 for skip-gram model)\n");
    printf("\nExamples:\n");
    printf(
        "./word2vec -train data.txt -output vec.txt -size 200 -window 5 -sample 1e-4 -lambda 0.5 -negative 5 -binary 0 -cbow 1 -iter 3\n\n");
    return 0;
  }
  output_file[0] = 0;
  save_vocab_file[0] = 0;
  read_vocab_file[0] = 0;
  if ((i = ArgPos((char *) "-size", argc, argv)) > 0) layer1_size = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-train", argc, argv)) > 0) strcpy(train_file, argv[i + 1]);
  if ((i = ArgPos((char *) "-save-vocab", argc, argv)) > 0) strcpy(save_vocab_file, argv[i + 1]);
  if ((i = ArgPos((char *) "-read-vocab", argc, argv)) > 0) strcpy(read_vocab_file, argv[i + 1]);
  if ((i = ArgPos((char *) "-debug", argc, argv)) > 0) debug_mode = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-binary", argc, argv)) > 0) binary = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-cbow", argc, argv)) > 0) cbow = atoi(argv[i + 1]);
  if (cbow) alpha = 0.05;
  if ((i = ArgPos((char *) "-alpha", argc, argv)) > 0) alpha = atof(argv[i + 1]);
  if ((i = ArgPos((char *) "-lambda", argc, argv)) > 0) lambda = atof(argv[i + 1]);
  if ((i = ArgPos((char *) "-output", argc, argv)) > 0) strcpy(output_file, argv[i + 1]);
  if ((i = ArgPos((char *) "-kappa", argc, argv)) > 0) strcpy(kappa_file, argv[i + 1]);
  if ((i = ArgPos((char *) "-window", argc, argv)) > 0) window = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-sample", argc, argv)) > 0) sample = atof(argv[i + 1]);
  if ((i = ArgPos((char *) "-negative", argc, argv)) > 0) negative = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-threads", argc, argv)) > 0) num_threads = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-iter", argc, argv)) > 0) iter = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-min-count", argc, argv)) > 0) min_count = atoi(argv[i + 1]);
  if ((i = ArgPos((char *) "-classes", argc, argv)) > 0) classes = atoi(argv[i + 1]);
  vocab = (struct vocab_word *) calloc(vocab_max_size, sizeof(struct vocab_word));
  vocab_hash = (int *) calloc(vocab_hash_size, sizeof(int));
  docs = (int *) calloc(corpus_max_size, sizeof(int));
  doc_sizes = (long long *) calloc(corpus_max_size, sizeof(long long));
  expTable = (real *) malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
  for (i = 0; i < EXP_TABLE_SIZE; i++) {
    expTable[i] = exp((i / (real) EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); // Precompute the exp() table
    expTable[i] = expTable[i] / (expTable[i] + 1);                   // Precompute f(x) = x / (x + 1)
  }
  TrainModel();
  return 0;
}