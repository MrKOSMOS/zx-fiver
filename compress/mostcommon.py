freq = {}

skip = ('abbas', 'zaire', 'rapes', 'raped', 'avant', 'busty', 'cocks', 'comix', 'corey', 'cowan', 'costa', 'danny', 'penis', 'diffs',
            'dildo', 'dixie', 'donna', 'dover', 'dunno', 'emacs', 'erica', 'fedex', 'foley', 'forex', 'fritz', 
            'fucks', 'garth', 'genet', 'genoa', 'gimme', 'gotta', 'greek', 'henry', 'india', 'infos', 'infra',
            'jerry', 'jenny', 'jesus', 'jesse', 'jimmy', 'johns', 'jones', 'judas', 'krona', 'krone', 'kylie', 
            'lacey', 'laura', 'layup', 'leone', 'lesbo', 'levin', 'lewis', 'linux', 'logan', 'louis', 'lowry',
            'lycra', 'mayan', 'mecca', 'milfs', 'molly', 'monde', 'mondo', 'monte', 'monty', 'morse', 'moses',
            'nancy', 'nazis', 'negro', 'nelly', 'norma', 'nudes', 'oscar', 'paddy', 'outta', 'ortho', 'panty',
            'paolo', 'paris', 'parma', 'pedro', 'peggy', 'penis', 'pepsi', 'perry', 'playa', 'polly', 'porno',
            'pratt', 'prima', 'pussy', 'quasi', 'randy', 'ralph', 'reiki', 'rhine', 'riley', 'roger', 'roman',
            'rowan', 'rubin', 'sakai', 'salle', 'sally', 'santo', 'savoy', 'scots', 'semen', 'sloan', 'sluts',
            'swiss', 'tammy', 'terra', 'terry', 'texas', 'thanx', 'tommy', 'tonga', 'tyler', 'vegas', 'waldo', 
            'wales', 'warez', 'welch', 'welsh', 'whore', 'willy', 'titty', 'fecal', 'urine', 'clits', 'gonzo',
            'platt', 'lexis', 'wicca', 'platt', 'tutti', 'thema', 'dykes', 'punto', 'pharm', 'scala', 'cours',
            'tesla', 'bebop', 'bitte', 'telco', 'shiva', 'brits', 'zippo', 'darcy', 'libra', 'merle', 'ovary',
            'ginny', 'merle', 'fonda', 'zorro', 'corso', 'potts', 
            )

with open("unigram_freq.csv") as u:
    u.readline()
    for line in u:
        word,f = line.strip().split(",")
        freq[word] = int(f)

use = []

with open("full.txt") as f:
    for line in f:
        if len(line)>=5:
            w = line.strip()
            if w in freq:
                use.append((w,freq[w]))

def sorter(x):
    return 0 if x[0] in skip else -x[1]
    
use.sort(key=sorter)

words = list(w[0] for w in use[:2315])
words.sort()

with open("answers.txt", "w") as a:
    for w in words:
        a.write(w+"\n")
                