all: sim find_route cross_valid
clear:
	rm ../sim
	rm ../find_route
	rm ../cross
sim:
	gcc fb_draw.c main.c readjpeg.c graph.c scheduler.c station.c -ljpeg -o ../sim
find_route:
	gcc graph.c find_routes.c fb_draw.c readjpeg.c -ljpeg -o ../find_route
cross_valid:
	gcc cross_valid.c scheduler.c graph.c fb_draw.c readjpeg.c station.c -ljpeg -o ../cross
