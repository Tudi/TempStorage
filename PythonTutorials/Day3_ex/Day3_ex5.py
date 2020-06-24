def filter_short_words(word_list, n):
    return list(filter(lambda x: len(x) < n, word_list))


words = ['abraca', 'dabra', 'a', 'b']
print(filter_short_words(words, 3))
