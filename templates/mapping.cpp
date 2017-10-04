int nr = 32, nt = 1;

// Initializes the mapping object map
void create_map(mapping& map) {
    map.set_ndomains(1);
    map.set_npts(nr);
    map.gl.set_xif(0., 1.); // zeta limits
    map.set_nt(nt);
    map.init();
}
