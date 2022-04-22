
fp = open('./ddmin_result', 'r')


lines = fp.readlines()

print('# \t times \t #')
for i, line in enumerate(lines):
    if 'last minimized: ./' in line:
        next_line = lines[i+1]
        pre = next_line.find('iteration: ') + len('iteration: ')
        print(next_line[pre:-1], end=', ')

        #prx = len('len: ')
        #pox = next_line.find(',')
        #print(next_line[prx:pox], end=', ')

        #prx = len('last minimized: ./')
        #pox = len('.part')+1
        #print(line[prx:-pox], end=', ')


'''
print('\n# \t characters \t #')
for line in lines:
    if 'len: ' in line:
        prx = len('len: ')
        pox = line.find(',')
        print(line[prx:pox], end=', ')
'''
