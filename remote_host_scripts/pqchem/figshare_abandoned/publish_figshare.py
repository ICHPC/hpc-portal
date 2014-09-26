#!/usr/bin/python
from optparse import OptionParser
import requests
from oauth_hook import OAuthHook
import json
from os.path import basename
from os.path import exists
from sys import exit


parser = OptionParser()
parser.add_option( "-f", "--file", dest="file" , action="append" )
parser.add_option( "-t", "--title", dest="title" )
parser.add_option( "-d", "--description", dest="description" )
parser.add_option( "-g", "--tag",dest="tag", action="append" )
parser.add_option( "-a", "--author",dest="author", action="append" )
parser.add_option( "-c", "--category",dest="category", action="append" )
parser.add_option( "-k",  "--key",dest="key" )
parser.add_option( "-s",  "--secret",dest="secret" )
parser.add_option( "-l",  "--link",dest="link", action="append" )

(options, args) = parser.parse_args()


for filename in options.file:
	if (False == exists( filename )) :
		exit( "File "+filename+" does not exist" )


if ( options.key )  :
	OAuthHook.consumer_key = options.key

if ( options.secret ) : 
	OAuthHook.consumer_secret = options.secret

oauth_hook = OAuthHook(header_auth=True)

client = requests.session(hooks={'pre_request': oauth_hook})

body = {'title':options.title, 'description':options.description,'defined_type':'fileset'}
headers = {'content-type':'application/json'}

response = client.post('http://api.figshare.com/my_data/articles',
                        data=json.dumps(body), headers=headers)

results = json.loads(response.content)

print json.dumps(body)
print headers
print results
article = results['article_id']

#for a in options.author:
#	print a
#	client = requests.session(hooks={'pre_request': oauth_hook})
#
#	body = {'full_name':a}
#	headers = {'content-type':'application/json'}
#
## Look up the author
#	response = client.get('http://api.figshare.com/my_data/authors?search_for=' + a)
#	results = json.loads(response.content)
#
## Author doesn't exist.. make them
#	if ( not 'items' in results  ) :
#		print "Creating author record"
#		response = client.post('http://api.figshare.com/my_data/authors',
#  	                      data=json.dumps(body), headers=headers)
#		results = json.loads(response.content)	
#
#	aid=results['items'][0]['id']
#	body = {'author_id':aid}
#	headers = {'content-type':'application/json'}
#
#	response = client.put('http://api.figshare.com/my_data/articles/' +str(article)+'/authors',
#                        data=json.dumps(body), headers=headers)
#	results = json.loads(response.content)




for filename in options.file: 
	print "Uploading file " + filename

	pubfilename=basename(filename)
	files = { 'filedata':( pubfilename , open( filename, 'rb')) }
	response = client.put('http://api.figshare.com/my_data/articles/'+str(article)+'/files', files=files)
	results = json.loads(response.content)
 
	print results


for tag in options.tag:
	print tag

	body = {'tag_name':tag }
	headers = {'content-type':'application/json'}
	response = client.put('http://api.figshare.com/my_data/articles/'+str(article)+'/tags', data=json.dumps(body), headers=headers)
	results = json.loads(response.content)

for category in options.category:
	print tag

	body = {'category_id':category }
	headers = {'content-type':'application/json'}
	response = client.put('http://api.figshare.com/my_data/articles/'+str(article)+'/categories', data=json.dumps(body), headers=headers)
	results = json.loads(response.content)

for link in options.link:
	print link
	body = {'link':link }
	headers = {'content-type':'application/json'}
	response = client.put('http://api.figshare.com/my_data/articles/'+str(article)+'/links', data=json.dumps(body), headers=headers)
	results = json.loads(response.content)
	print results



response = client.post('http://api.figshare.com/my_data/articles/' + str(article) + '/action/make_public')
results = json.loads(response.content)
print results

print "DOI 10.6084/m9.figshare." + str(article)
