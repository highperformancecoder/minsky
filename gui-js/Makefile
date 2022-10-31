compile-ts: build-web build-electron

build-web:
	npm run build:web| sed -e 's/\x1b\[[0-9;]*m//g'|sed -e 's/(\([0-9]*\),[0-9]*)/:\1/g'|sed -e 's/ERROR/error/g'

build-electron:
	npm run build:electron| sed -e 's/\x1b\[[0-9;]*m//g'|sed -e 's/(\([0-9]*\),[0-9]*)/:\1/g'|sed -e 's/ERROR/error/g'

