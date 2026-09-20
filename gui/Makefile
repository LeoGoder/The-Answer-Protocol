docker:
	docker compose run --rm compiler bash

pcsx2:
	./squashfs-root/AppRun "$(PWD)/tyra/tap/bin/tap.elf"

lint:
	cpplint --filter=-legal/copyright --recursive .

clean:
	rm -fr raylib/
	rm -fr ps2gl/
