from boto.s3.connection import S3Connection
from boto.s3.key import Key
from progressbar import ProgressBar
import sys
import os
import time

if len(sys.argv) < 2:
	print "Usage: upload_thirdparty.py <file.tgz>"
	exit()
else:
	fname = sys.argv[1]

acctid = raw_input("AWS_ACCESS_KEY_ID: ")
secret = raw_input("AWS_SECRET_ACCESS_KEY: ")
bucket = "kroll.appcelerator.com"
key = os.path.basename(fname)
conn = S3Connection(acctid, secret)
bucket = conn.get_bucket(bucket)
k = Key(bucket)
k.key = key

pbar = ProgressBar().start()
try:
	def progress_callback(current, total):
		pbar.update(int(100 * (float(current) / float(total))))
	k.set_contents_from_filename(fname, cb=progress_callback, num_cb=100)
finally:
	pbar.finish()

k.set_acl('public-read')
