<?php
$input = "in(0,23) in(1,22) in(2,27) in(3,13) in(6,24) in(7,9) in(8,6) in(9,1) in(10,7) in(11,15) in(12,19) in(13,26) in(15,17) in(16,2) in(17,3) in(18,29) in(19,25) in(21,18) in(22,21) in(23,8) in(24,12) in(25,16) in(26,10) in(27,11) r(24) r(9) r(25) r(12) r(23) r(11) r(15) r(22) r(8) r(6) r(19) r(10) r(16) r(21) r(1) r(17) r(2) r(3) r(18) r(26) r(27) r(29) r(7) r(13) v(0) v(1) v(2) v(3) v(4) v(5) v(6) v(7) v(8) v(9) v(10) v(11) v(12) v(13) v(14) v(15) v(16) v(17) v(18) v(19) v(20) v(21) v(22) v(23) v(24) v(25) v(26) v(27) v(28) v(29) e(0,24,1) e(0,9,8) e(0,25,4) e(0,12,2) e(0,23,8) e(0,11,8) e(1,18,6) e(1,22,5) e(1,24,2) e(1,29,9) e(2,20,8) e(2,29,9) e(2,27,9) e(2,8,7) e(2,6,5) e(3,0,4) e(3,13,6) e(3,9,4) e(3,5,2) e(3,4,5) e(3,27,2) e(4,0,2) e(4,26,2) e(4,27,8) e(5,25,1) e(5,23,2) e(5,15,3) e(6,3,8) e(6,27,6) e(6,11,1) e(6,24,7) e(7,20,5) e(7,9,7) e(7,14,7) e(8,5,3) e(8,6,4) e(9,6,4) e(9,1,2) e(9,4,4) e(10,7,7) e(10,8,4) e(10,16,4) e(11,15,7) e(12,24,1) e(12,20,4) e(12,6,9) e(12,19,7) e(12,23,5) e(12,10,3) e(13,15,5) e(13,22,2) e(13,23,3) e(13,27,2) e(13,9,3) e(13,26,5) e(14,26,4) e(15,10,6) e(15,11,3) e(15,17,6) e(16,15,1) e(16,22,7) e(16,6,4) e(16,7,6) e(16,2,7) e(16,29,5) e(17,2,9) e(17,10,2) e(17,3,9) e(17,20,5) e(17,18,2) e(17,9,7) e(17,22,3) e(18,0,2) e(18,29,6) e(18,9,4) e(19,25,9) e(19,2,2) e(19,0,6) e(19,24,8) e(19,20,3) e(21,22,8) e(21,19,2) e(21,24,4) e(21,18,5) e(21,23,2) e(21,8,5) e(22,0,4) e(22,6,5) e(22,18,7) e(22,21,8) e(23,22,3) e(23,8,7) e(23,20,4) e(24,23,9) e(24,17,2) e(24,12,9) e(24,21,5) e(24,9,3) e(25,16,9) e(25,21,4) e(25,20,2) e(25,24,8) e(25,19,1) e(26,10,7) e(26,22,8) e(27,11,5) e(27,13,4) e(27,0,7) e(28,10,8) e(28,5,3) e(28,9,5) e(28,11,2) e(28,18,1) e(29,12,7) e(29,7,3) e(29,3,5) source(0) r(0) target(29) _one_2_r(24) _one_4_r(24) _one_0_r(9) _one_2_r(9) _one_4_r(9) _one_1_r(25) _one_2_r(12) _one_3_r(12) _one_1_r(11) _one_2_r(11) _one_1_r(15) _one_2_r(15) _one_3_r(15) _one_0_r(22) _one_2_r(22) _one_3_r(22) _one_4_r(22) _one_4_r(8) _one_2_r(6) _one_2_r(19) _one_3_r(19) _one_4_r(19) _one_0_r(10) _one_3_r(10) _one_4_r(10) _one_1_r(16) _one_4_r(16) _one_0_r(21) _one_1_r(21) _one_0_r(1) _one_2_r(1) _one_3_r(1) _one_1_r(17) _one_2_r(17) _one_3_r(17) _one_4_r(17) _one_1_r(2) _one_3_r(2) _one_0_r(3) _one_0_r(18) _one_1_r(18) _one_4_r(18) _one_0_r(26) _one_3_r(26) _one_1_r(27) _one_3_r(27) _one_4_r(27) _one_0_r(7) _one_2_r(7) _one_0_r(13) _one_4_r(13)";
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