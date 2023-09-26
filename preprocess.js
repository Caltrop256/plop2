const fs = require('fs');
const files = fs.readdirSync('./src/c/elements/');
const joinImages = require('join-images').default;

const elementTypes = [];
const dataStructs = new Map();

let out = `// DONT EDIT THIS CODE DIRECTLY!

#ifndef ELEMENTDATA_H
#define ELEMENTDATA_H

#include "../main.h"

#define FOREACH_ELEMENTTYPE(M)\\\n`
for(const dir of files) {
    if(fs.statSync(`./src/c/elements/${dir}`).isDirectory()) {
        const files = fs.readdirSync(`./src/c/elements/${dir}`);
        for(const file of files) {
            const name = file.substring(0, file.length - 2);
            elementTypes.push(name);
            out += `    M(${name.toUpperCase()},${name.toLowerCase()})\\\n`;

            const contents = fs.readFileSync(`./src/c/elements/${dir}/${file}`, {encoding: 'utf-8'});
            const ind = contents.search(`struct data {`);
            if(ind != -1) {
                let braceCount = 1;
                let struct = `struct data {`;
                let i = ind + struct.length;
                while(braceCount) {
                    let char = contents.charAt(i++);
                    struct += char;
                    if(char == '{') braceCount += 1;
                    else if(char == '}') braceCount -= 1;
                }
                dataStructs.set(name, struct);
            }
        }
    }
}

out += `
typedef enum ElementType {
    TYPE_EMPTY,
    #define GENERATE_ENUM(ENUM,_) TYPE_##ENUM,
    FOREACH_ELEMENTTYPE(GENERATE_ENUM)
    #undef GENERATE_ENUM
    type_length
} __attribute__((__packed__)) ElementType;
`

out += `
#define FOREACH_ELEMENTTYPE_WITH_STRUCT_DATA(M)\\
${Array.from(dataStructs).map(([type]) => `    M(${type.toUpperCase()},${type.toLowerCase()})`).join('\\\n')}

${Array.from(dataStructs).map(([type, struct]) => {
    return `${struct.replace(`struct data {`, `struct ${type}_data {`)};`
}).join('\n')}

#endif
`;

fs.writeFileSync('./src/c/elements/elementdata.h', out, {encoding: 'utf-8'})

const atlas = elementTypes.map(t => {
    const path = `./src/textures/${t}.png`;
    if(fs.existsSync(path)) return path;
    else return './src/textures/missing.png'
});
while(Math.log2(atlas.length) % 1 != 0) atlas.push('./src/textures/missing.png');

joinImages(atlas).then(img => img.toFile('./src/static/atlas.png'));