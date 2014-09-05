#!/bin/bash

while true; do
	echo > game
	./main
	game=$(<game)

	echo $game > oldgame

	if [ -z "$game" ]; then
	    exit 0
	fi

	./games/$game/launch.sh
done
