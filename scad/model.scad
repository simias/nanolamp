$fn = 50;

union() {
	difference() {
		union() {
			difference() {
				cube(size = [101, 135, 55]);
				translate(v = [10, -5, 45]) {
					cube(size = [81, 145, 55]);
				}
				translate(v = [15, 8, 40]) {
					cube(size = [71, 119, 55]);
				}
				translate(v = [20, 13, -1]) {
					cube(size = [61, 109, 55]);
				}
			}
			%translate(v = [16, 9, 40.01]) {
				cube(size = [69, 117, 2]);
			}
		}
		translate(v = [29, 105, 17.0]) {
			rotate(a = [0, 90, 180]) {
				union() {
					cylinder(h = 20, r1 = 12.0, r2 = 12.0);
					translate(v = [0, 0, 19]) {
						cylinder(h = 21, r1 = 4.5, r2 = 4.5);
					}
					#translate(v = [0, 0, 30]) {
						cylinder(h = 16, r1 = 7.0, r2 = 7.0);
					}
				}
			}
		}
		translate(v = [7, 80, 11.5]) {
			rotate(a = [-90, 180, 90]) {
				union() {
					cube(size = [61.0, 11.0, 8]);
					translate(v = [0, 0, -25]) {
						translate(v = [1, 1, 1]) {
							minkowski() {
								cube(size = [9.0, 9.0, 28]);
								sphere(r = 1);
							}
						}
					}
					translate(v = [50, 0, -25]) {
						translate(v = [1, 1, 1]) {
							minkowski() {
								cube(size = [9.0, 9.0, 28]);
								sphere(r = 1);
							}
						}
					}
					#translate(v = [0.5, 0.5, 0.5]) {
						cube(size = [60, 10, 8.5]);
					}
					#translate(v = [20, 5.0, 7]) {
						cube(size = [5, 1, 15]);
					}
				}
			}
		}
	}
	%translate(v = [10.5, -27.5, 45.01]) {
		cube(size = [80, 190, 190]);
	}
}
