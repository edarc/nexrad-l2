import math

# IMPORTANT: Angular values not stored in variables suffixed "_deg" are in
# radians.

TILE_DIMENSION_PIXELS = 256

def apply_raster_transform(x, y, sphere_circumference_pixels):
    """
    Apply the pixel coordinate system transform. 

    Un-transformed pixel coordinates from the Mercator projection use an origin
    at the equator and the prime meridian, with increasing y pointing north.
    The raster system used in the tiles has the origin at slightly north of
    +85deg and the international date line, with increasing y pointing south
    (as one expects in a raster display).

    This function applies that transformation.
    """
    false_easting = -1.0 * sphere_circumference_pixels / 2.0
    false_northing = sphere_circumference_pixels / 2.0

    return (x - false_easting, false_northing - y)

def inverse_raster_transform(x, y, sphere_circumference_pixels):
    """
    Apply the inverse pixel coordinate system transform.

    Transforms tile pixel coordinates into coordinates that can be used in the
    inverse Mercator projection to get lat/lons. See
    `apply_projection_transform`.
    """
    false_easting = -1.0 * sphere_circumference_pixels / 2.0
    false_northing = sphere_circumference_pixels / 2.0

    return (x + false_easting, false_northing - y)

def ll_to_pixel_mercator(lat_deg, lon_deg, zoomlevel):
    """
    Maps a lat/lon (in decimal degrees) to the Google Maps pixel-mercator
    projection at the given zoomlevel. It returns the following structure::

      ((t_x, t_y), (dt_x, dt_y))

    where `t_x` and `t_y` are the tile coordinate at that zoomlevel, and `dt_x`
    and `dt_y` are the actual pixel coordinates from the origin of the given
    tile.
    """
    tiles_per_side = 2 ** zoomlevel

    sphere_circumference_pixels = tiles_per_side * TILE_DIMENSION_PIXELS
    sphere_radius_pixels = sphere_circumference_pixels / (2 * math.pi)

    lat = (lat_deg * math.pi) / 180
    lon = (lon_deg * math.pi) / 180

    # Apply the Mercator projection
    r = sphere_radius_pixels
    projected_x = r * lon
    projected_y = (r / 2) * math.log(
            (1.0 + math.sin(lat)) / (1.0 - math.sin(lat)) )

    # Apply the raster transform
    zoomed_x, zoomed_y = apply_raster_transform(projected_x, projected_y,
            sphere_circumference_pixels)

    # Get tile location and offsets
    t_x, dt_x = ((zoomed_x // TILE_DIMENSION_PIXELS),
            (zoomed_x % TILE_DIMENSION_PIXELS))
    t_y, dt_y = ((zoomed_y // TILE_DIMENSION_PIXELS),
            (zoomed_y % TILE_DIMENSION_PIXELS))

    return ((int(t_x), int(t_y)), (dt_x, dt_y))

def pixel_mercator_to_ll(t_x, t_y, dt_x, dt_y, zoomlevel):
    """
    Maps a tile location/offset into lat/lon. Returns the following structure::

      (lat, lon)
    """
    tiles_per_side = 2 ** zoomlevel

    sphere_circumference_pixels = tiles_per_side * TILE_DIMENSION_PIXELS
    sphere_radius_pixels = sphere_circumference_pixels / (2 * math.pi)

    # Find absolute pixel coordinates
    zoomed_x = t_x * TILE_DIMENSION_PIXELS + dt_x
    zoomed_y = t_y * TILE_DIMENSION_PIXELS + dt_y

    # Inverse raster transform
    projected_x, projected_y = inverse_raster_transform(zoomed_x, zoomed_y,
            sphere_circumference_pixels)

    # Inverse mercator projection
    r = sphere_radius_pixels
    lat = (math.pi / 2.0) - (2.0 * math.atan( math.exp(-1.0 * projected_y / r) ))
    lon = projected_x / r

    return (lat, lon)
