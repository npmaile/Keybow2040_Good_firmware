.PHONY: build-extract-docker
build-extract-docker: 
	docker build -t 2040fw:latest .
	docker create --name temp-container "2040fw:latest"
	docker cp "temp-container:/root/test.uf2" ./2040fw.uf2
	docker rm temp-container

