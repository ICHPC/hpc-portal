#!/usr/local/bin/python
from optparse import OptionParser
import requests
from oauth_hook import OAuthHook
import json
from os.path import basename
from os.path import exists
from sys import exit


parser = OptionParser()
parser.add_option( "-k",  "--consumerkey",dest="consumerkey" )
parser.add_option( "-s",  "--consumersecret",dest="consumersecret" )
parser.add_option( "-K",  "--tokenkey",dest="tokenkey" )
parser.add_option( "-S",  "--tokensecret",dest="tokensecret" )
parser.add_option( "-a", "--article",dest="article", action="append" )

(options, args) = parser.parse_args()


if ( options.consumerkey )  :
	OAuthHook.consumer_key = options.consumerkey

if ( options.consumersecret ) : 
	OAuthHook.consumer_secret = options.consumersecret

oauth_hook = OAuthHook( options.tokenkey, options.tokensecret, header_auth=True)

client = requests.session(hooks={'pre_request': oauth_hook})

if ( options.article ):
	for  article in options.article :
		response = client.post('http://api.figshare.com/v1/my_data/articles/'+str(article)+'/action/make_public')
		print "DOI: 10.6084/m9.figshare." + str(article)

