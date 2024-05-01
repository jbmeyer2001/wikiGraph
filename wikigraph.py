import bz2
from bs4 import BeautifulSoup
import httpx
import os
import re

WIKIDUMPS = 'https://dumps.wikimedia.org/'

# Create an HTTP/2 client
client = httpx.Client(http2=True)

#query wikimedia enwiki for english language database dumps
wikidump_response = client.get(WIKIDUMPS + 'enwiki/')
if wikidump_response.status_code != 200:
    raise Exception('http request responded with code ' + wikidump_response.status_code + ', expecting 200')

#find the folder for the latest version on wikimedia
wikidump_soup = BeautifulSoup(wikidump_response.text, "lxml")
links = list(map(lambda x: x.get('href').replace('/', ''), wikidump_soup.find_all('a')))
links.remove('..')
links.remove('latest')
latest_date = str(max(list(map(lambda x: int(x), links))))
download_pages_link = WIKIDUMPS + 'enwiki/' + latest_date + '/'

download_page = client.get(download_pages_link)
download_soup = BeautifulSoup(download_page.content, 'lxml')
files = download_soup.find_all('li', {'class': 'file'})

download_links = []
for file in files:
    file_str = str(file)
    #we want to download the 'enwiki-[date]-pages-articles-multistream' files, however we don't want the index .txt files, nor the non-numbered ...multistream.xml file
    if file_str.find('-pages-articles-multistream') != -1 and file_str.find('.txt') == -1 and file_str.find('multistream.xml') == -1:
        download_links.append(file.find('a').get('href'))

#for link in download_links:
link = download_links[0]
print('downloading ' + link)
response = client.get(WIKIDUMPS + link)
print('decompressing')
data = bz2.decompress(response.content)
print('interpreting as xml')
pages = list(BeautifulSoup(data, 'lxml').find_all('page'))

with open('test.txt', 'w', encoding='utf8') as file:
    for page in pages:
        title = page.find('title').text.title()
        redirect = page.find('redirect')
        if redirect != None:
            redirect_str = str(redirect)
            redirect_str = redirect_str[redirect_str.find('"') + 1:redirect_str.rfind('"')]
            file.write('redirect ' + title + ' to ' + redirect_str + '\n')
        else:
            #use regular expression to find the text within double brackets ex: [[this text]]
            reg = set(re.findall('\[\[(((?!\[)(?!\]).)+)\]\]', str(page.text)))
            hyperlinks = set()
            for tuple in reg:
                hyperlink = tuple[0].title()
                hyperlink = hyperlink.replace(' ', '_')

                if hyperlink.find('|') != -1:
                    hyperlink = hyperlink[:hyperlink.find('|')]

                if hyperlink.find(':') == -1:
                    hyperlinks.add(hyperlink)
            
            file.write(title + ' Contains hyperlinks to: [')
            for link in hyperlinks:
                file.write(link + ', ')
            file.write(']\n')
    file.close()


