import sys
from html.parser import HTMLParser

class MyHTMLParser(HTMLParser):
    def __init__(self):
      super().__init__()
    def handle_starttag(self, tag, attrs):
      if "a" == tag and len(attrs[0]) == 2 and "href" == attrs[0][0]:
        print(attrs[0][1])
        exit(0)

parser = MyHTMLParser()
parser.feed(sys.argv[1])
exit(1)
