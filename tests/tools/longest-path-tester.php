<?php
$input = "in(25,16) in(24,12) in(23,8) in(22,21) in(21,22) in(19,25) in(18,9) in(17,3) in(4,27) r(13) r(14) in(3,4) r(7) r(27) r(26) r(18) r(3) in(6,24) r(10) in(11,15) in(14,26) in(13,23) r(21) r(17) in(12,19) r(16) in(16,2) r(11) r(6) r(22) in(15,17) r(15) in(10,7) r(8) in(9,1) in(8,6) r(2) r(1) in(7,14) in(2,29) in(1,18) r(4) r(19) in(0,11) in(26,10) in(27,13) r(24) r(9) r(25) r(12) r(23) r(29) e(19,0,6) e(19,2,2) e(19,25,9) e(18,9,4) e(18,29,6) e(18,0,2) e(17,22,3) e(17,9,7) e(17,18,2) e(17,20,5) e(17,3,9) e(17,10,2) e(17,2,9) e(16,29,5) e(16,2,7) e(16,7,6) e(16,6,4) e(16,22,7) e(16,15,1) e(15,17,6) e(15,11,3) e(15,10,6) e(14,26,4) e(13,26,5) e(13,9,3) e(13,27,2) e(13,23,3) e(13,22,2) e(13,15,5) e(12,10,3) e(12,23,5) e(12,19,7) e(12,6,9) e(12,20,4) e(12,24,1) e(11,15,7) e(10,16,4) e(10,8,4) e(10,7,7) e(9,4,4) e(9,1,2) e(23,22,3) e(23,8,7) e(23,20,4) e(24,23,9) v(0) e(24,17,2) v(1) e(24,12,9) v(2) e(24,21,5) v(3) e(24,9,3) v(4) e(25,16,9) v(5) e(25,21,4) v(6) e(25,20,2) v(7) e(25,24,8) v(8) e(25,19,1) v(9) e(26,10,7) v(10) e(26,22,8) v(11) e(27,11,5) v(26) v(25) v(24) v(14) v(17) e(28,11,2) v(23) v(13) v(16) e(29,12,7) e(28,9,5) v(22) v(12) v(15) e(28,5,3) target(29) v(21) e(28,10,8) r(0) v(20) e(27,0,7) e(27,13,4) e(22,21,8) source(0) v(19) e(22,18,7) e(29,3,5) v(18) e(22,6,5) e(29,7,3) e(22,0,4) e(21,8,5) e(28,18,1) e(21,23,2) e(21,18,5) e(21,24,4) e(21,19,2) e(21,22,8) e(19,20,3) e(19,24,8) v(27) v(28) v(29) e(0,24,1) e(0,9,8) e(0,25,4) e(0,12,2) e(0,23,8) e(0,11,8) e(1,18,6) e(1,22,5) e(1,24,2) e(1,29,9) e(2,20,8) e(2,29,9) e(2,27,9) e(2,8,7) e(2,6,5) e(3,0,4) e(3,13,6) e(3,9,4) e(3,5,2) e(3,4,5) e(3,27,2) e(4,0,2) e(4,26,2) e(4,27,8) e(5,25,1) e(5,23,2) e(5,15,3) e(6,3,8) e(6,27,6) e(6,11,1) e(6,24,7) e(7,20,5) e(7,9,7) e(7,14,7) e(8,5,3) e(8,6,4) e(9,6,4)";
$atoms = explode(" ", $input);
$edges = array_map(function($atom) {
    return explode(",", substr($atom, 2, -1));
}, array_filter($atoms, function($atom) {
    return str_starts_with($atom, "e(");
}));
$path = array_map(function($atom) {
    return explode(",", substr($atom, 3, -1));
}, array_filter($atoms, function($atom) {
    return str_starts_with($atom, "in(");
}));

$source = 0;
$target = 29;

$distance = 0;
$current_node = $source;
while ($current_node != $target) {
    foreach ($path as $p) {
        if ($p[0] == $current_node) {
            $found = false;
            foreach ($edges as $e) {
                if ($e[0] == $current_node && $e[1] == $p[1]) {
                    $distance += $e[2];
                    $found = true;
                    break;
                }
            }
            if (!$found) {
                throw new Exception("Edge " . $current_node . " -> " . $p[1] . " not found");
            }
            $current_node = $p[1];
            break;
        }
    }
}

echo $distance . PHP_EOL;