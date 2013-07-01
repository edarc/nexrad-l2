<?php
$GENERATOR_PROG = "/home/kyle/nexrad-processor/bin/gcc-4.3.2/release/gen-one";
$TILE_PREFIX = "/var/www/vhosts/neowx.com/httpdocs/radar/tiles";
$BASE_PREFIX = "/home/kyle/nexrad-processor/basefiles";

$site = $_GET['site'];
$t_x = $_GET['x'];
$t_y = $_GET['y'];
$t_z = $_GET['z'];

$base_file = "${BASE_PREFIX}/${site}.base";
$tile_file = "${TILE_PREFIX}/${site}_${t_z}_${t_x}-${t_y}.png";

if (!is_numeric($t_x) || !is_numeric($t_y) || !is_numeric($t_z)
    || !ereg('^K[A-Z]{3}$', $site))
{
    // The input parameters are bogus.
    header("HTTP/1.1 400 Bad Request");
    exit;
}

clearstatcache();
if (file_exists($tile_file)
    && (filemtime($tile_file) > filemtime($base_file)))
{
    // We generated this tile already and it's still up-to-date
    header("X-Sendfile: ${tile_file}");
    header("Content-Type: image/png");
    header("X-NeoWX-Radar: cached");
}
else
{
    // Invoke the tile generator to generate this tile
    exec("${GENERATOR_PROG} ${base_file} ${t_x} ${t_y} ${t_z} ${tile_file}",
        $output_arr);
    $output = join("", $output_arr);

    if ($output == "200")
    {
        // If it prints the string "200", the tile is in the coverage area and 
        // was generated.
        header("X-Sendfile: ${tile_file}");
        header("Content-Type: image/png");
	header("X-NeoWX-Radar: generated");
    }
    else if ($output == "404")
    {
        // If it prints the string "404", the tile is outside the radar's 
        // coverage area and nothing was generated, so we 404.
        header("HTTP/1.1 404 Not Found");
	header("X-NeoWX-Radar: no-coverage");
    }
    else
    {
        // Something weird happened
        header("HTTP/1.1 500 Internal Server Error");
	echo $output;
    }
}

?>
