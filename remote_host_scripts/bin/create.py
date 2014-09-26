#!/usr/bin/python
# vim: set noexpandtab
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
parser.add_option( "-k",  "--consumerkey",dest="consumerkey" )
parser.add_option( "-s",  "--consumersecret",dest="consumersecret" )
parser.add_option( "-K",  "--tokenkey",dest="tokenkey" )
parser.add_option( "-S",  "--tokensecret",dest="tokensecret" )
parser.add_option( "-u", "--url",dest="url", action="append" )
parser.add_option( "-p", "--project",dest="project", action="append" )

(options, args) = parser.parse_args()

for filename in options.file:
	if (False == exists( filename )) :
		exit( "File "+filename+" does not exist" )

#OAuthHook.consumer_key = '97493_9GR0VB'
#OAuthHook.consumer_secret = '54NNRMJE18VU4DA1MGVQ'

if ( options.consumerkey )  :
	OAuthHook.consumer_key = options.consumerkey

if ( options.consumersecret ) : 
	OAuthHook.consumer_secret = options.consumersecret

#print "TOKENS:"
#print options.consumerkey
#print options.consumersecret
#print options.tokenkey
#print options.tokensecret


oauth_hook = OAuthHook( options.tokenkey, options.tokensecret, header_auth=True)

client = requests.session(hooks={'pre_request': oauth_hook})

#print options.description
#print options.title

body = {'title':options.title, 'description':options.description,'defined_type':'fileset'}
headers = {'content-type':'application/json'}

response = client.post('http://api.figshare.com/v1/my_data/articles',
                        data=json.dumps(body), headers=headers)


results = json.loads(response.content)
#print results
article = results['article_id']

#for a in options.author:
#	print a
#	client = requests.session(hooks={'pre_request': oauth_hook})
#
#	body = {'full_name':a}
#	headers = {'content-type':'application/json'}
#
## Look up the author
#	response = client.get('http://api.figshare.com/v1/my_data/authors?search_for=' + a)
#	results = json.loads(response.content)
#
## Author doesn't exist.. make them
#	if ( not 'items' in results  ) :
#		print "Creating author record"
#		response = client.post('http://api.figshare.com/v1/my_data/authors',
#  	                      data=json.dumps(body), headers=headers)
#		results = json.loads(response.content)	
#
#	aid=results['items'][0]['id']
#	body = {'author_id':aid}
#	headers = {'content-type':'application/json'}
#
#	response = client.put('http://api.figshare.com/v1/my_data/articles/' +str(article)+'/authors',
#                        data=json.dumps(body), headers=headers)
#	results = json.loads(response.content)




for filename in options.file: 
	#print "Uploading file " + filename

	pubfilename=basename(filename)
	files = { 'filedata':( pubfilename , open( filename, 'rb')) }
	response = client.put('http://api.figshare.com/v1/my_data/articles/'+str(article)+'/files', files=files)
	results = json.loads(response.content)
 
	#print results

if( options.url ):
 for url in options.url:
	#print url
	body = {'link':url }
	headers = {'content-type':'application/json'}

	response = client.put('http://api.figshare.com/v1/my_data/articles/'+str(article)+'/links',
                        data=json.dumps(body), headers=headers)
	results = json.loads(response.content)

if( options.tag):
 for tag in options.tag:
	#print tag

	body = {'tag_name':tag }
	headers = {'content-type':'application/json'}
	response = client.put('http://api.figshare.com/v1/my_data/articles/'+str(article)+'/tags', data=json.dumps(body), headers=headers)
	results = json.loads(response.content)

if( options.category ):
 for category in options.category:
	#print tag

	body = {'category_id':category }
	headers = {'content-type':'application/json'}
	response = client.put('http://api.figshare.com/v1/my_data/articles/'+str(article)+'/categories', data=json.dumps(body), headers=headers)
	results = json.loads(response.content)

#response = client.post('http://api.figshare.com/v1/my_data/articles/'+str(article)+'/action/make_public')


if ( options.project ):
 for  project in options.project :
	project_id=-1
# First look up project
	headers = {'content-type':'application/json'}
	response = client.get('http://api.figshare.com/v1/my_data/projects', headers=headers)
	results = (json.loads( response.content ));
#	print results
	if( results ):
		for p in results:
			if( p['title'] == project  and p['owner'] ):
				project_id= p['id']
				break

	if ( project_id == -1 ):
		#	printf( "MAKING A PROJECT %s\n", project )
		body = { 'title':project, "description":"Portal Project" }
		headers = {'content-type':'application/json'}
		response = client.post('http://api.figshare.com/v1/my_data/projects', data=json.dumps(body), headers=headers)
		results = json.loads(response.content)
#		print results
        if 'project_id' in results:
		  project_id=results['project_id']

	if( project_id != -1 ):
		body = { 'article_ids':[str(article)]  }
		headers = {'content-type':'application/json'}
		uri='http://api.figshare.com/v1/my_data/projects/'+str(project_id)+"/articles"
#		print uri
#		print json.dumps(body)
		response = client.put('http://api.figshare.com/v1/my_data/projects/'+str(project_id)+"/articles", data=json.dumps(body), headers=headers)
#	print "ADDED ARTICLE TO PROJECT"
#		print response
	

	

#print "DOI: 10.6084/m9.figshare." + str(article)
print "DOI: "+str(article);
