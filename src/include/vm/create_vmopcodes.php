<?php

/*
 * This script creates a vmopcodes.h include file from our vm_codes template. This is needed because we need to have
 * a few different kind of accessing the opcodes. Either by a define, or through an array (by hexidecimal code).
 */

// Read opcodes
$opcodes = array();
foreach (file($argv[1]) as $lineno => $line) {
    $line = trim($line);

    // Skip remarks
    if (empty($line) || $line[0] == ';') continue;

    // Skip uncorrectly formated codes
    if (! preg_match("/[\t ]*([A-Z_0-9]+)[\t ]+([0-9A-Fx]+)/i", $line, $match)) {
        echo "Encountered incorrect formated opcode at line $lineno\n";
        exit(1);
    }

    $opcodes[$match[2]] = $match[1];
}




/*
 * Write H file
 */

$fp = fopen($argv[2], "w");


$header = <<< EOH
/*
 Copyright (c) 2012, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __VM_OPCODES_H__
#define __VM_OPCODES_H__


    /*
     * WARNING: THIS FILE IS AUTOGENERATED! PLEASE USE CREATE_VMOPCODES_H.PHP TO REGENERATE!
     */


EOH;
fwrite($fp, $header);

// Do defines
foreach ($opcodes as $hex => $str) {
    fwrite($fp, sprintf("    #define VM_%-20s%s\n", strtoupper($str), $hex));
}
fwrite($fp, "\n\n");

fwrite($fp, "    int vm_codes_offset[256];\n");
fwrite($fp, "    char *vm_code_names[".count($opcodes)."];\n");
fwrite($fp, "    int vm_codes_offset[256];\n");

$footer = <<< EOT
#endif
EOT;
fwrite($fp, $footer);
fclose($fp);





// Write C file


$fp = fopen($argv[3], "w");

$header = <<< EOH
/*
 Copyright (c) 2012, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

EOH;
fwrite($fp, $header);



// Do indexes
fwrite($fp, "int vm_codes_index[".count($opcodes)."] = {\n");
fwrite($fp, "    ");
$s = join(", ", array_keys($opcodes));
fwrite($fp, wordwrap($s, 75, "\n    "));
fwrite($fp, "\n};\n");
fwrite($fp, "\n\n");

// Do offsets
fwrite($fp, "int vm_codes_offset[256] = {\n");
fwrite($fp, "    ");
$tmp = array();
for ($i=0; $i!=256; $i++) {
    $j = array_search(sprintf("0x%02X", $i), array_keys($opcodes));
    $tmp[] = $j !== false ? $j : -1;
    $s = join(", ", $tmp);
}
fwrite($fp, wordwrap($s, 75, "\n   "));
fwrite($fp, "\n};\n");
fwrite($fp, "\n\n");


// Do names
fwrite($fp, "char *vm_code_names[".count($opcodes)."] = {\n");
fwrite($fp, "    ");
$f = function($n) { return '"'.$n.'"'; };
$s = join(", ", array_map($f, $opcodes) );
fwrite($fp, wordwrap($s, 75, "\n    "));
fwrite($fp, "\n};\n");
fwrite($fp, "\n\n");


fclose($fp);