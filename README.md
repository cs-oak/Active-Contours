# Active-Contours

Various models of active contours. Windows application which loads PNM format images in greyscale (will convert automatically.)

To initialize a rubber-band contour, click and hold left click and drag. A closed on partially closed shape is recommended.
Upon releasing left click, the contour will down-sample appropriately. Press 3 to view the contour close in on an object.

To initialize a balloon contour, right click any location on the image. On releasing the right click, a small circle will
initialize. Press 4 to view the contour expand towards the edges it detects.

Once the rubber band contour is drawn, you may click on any point in the contour and drag it away. Pressing 2 now will start
another custom active contour algorithm which will re-shape the rest of the contour with respect to this point.

To Do: The point-and-drag method currently requires a very precise click, Need to change this to look in an area around the
pixel to be dragged. Previous attempt resulted in a crash when 2 pixels of interest were in the search area.
