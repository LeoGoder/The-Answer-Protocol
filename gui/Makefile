docker:
	docker compose run --rm compiler bash

pcsx2:
	cd tyra/tap/bin
	/home/lgoderne/sgoinfre/pcsx2/squashfs-root/AppRun "$PWD/tap.elf"

lint:
	cpplint --filter=-legal/copyright --recursive .

clean:
	rm -fr raylib/
	rm -fr ps2gl/
