/*global process,module,require,console*/

/* Palindrome Dictionary Finder
 * ============================
 * See `./a1.pdf` for details on how this program should operate.
 * Please note this is a prototype.
 * 
 * USAGE: `cat words | node palin.js`
 */

var main,
    TrieNode;

function main() {
    'use strict';
    var root = new TrieNode(''),
        items = [];
    
    // Encourage STDIN to behave desirably.
    process.stdin.resume();
    process.stdin.setEncoding('utf8');
    
    // Populate `items` from STDIN.
    process.stdin.on('data', function (data) {
        // Aggregate in the items.
        items = items.concat(data.split("\n"));
    });
    
    // Process the data.
    process.stdin.on('end', function () {
        // Add all the items to the Trie.
        items.map(root.add, root);
        // Go through all items and find reverses.
        items.map(function (val) {
            return val.split("").reverse().join("");
        }).filter(root.find, root).map(function (val) {
            console.log(val);
        });
    });
    
}

// Trie Node.
TrieNode = function TrieNode(value) {
    'use strict';
    
    this.value = value;
    this.children = {};
    this.word = false;

    return this;
};
// Add a new item to the Trie.
TrieNode.prototype.add = function add(item) {
    'use strict';
    var head,
        tail;
    
    switch (item.length) {
    case 0:
        // Base Case
        this.word = true;
        break;
    default:
        // Recursive Case
        head = item.charAt(0);
        tail = item.substring(1, item.length);
        
        switch (this.children[head]) {
        case null:
        case undefined:
            // No node created yet!
            // Create, then recursively add.
            this.children[head] = new TrieNode(head);
            this.children[head].add(item.substring(1, item.length));
            break;
        default:
            // A node exists.
            // Recursively add.
            this.words += 1;
            this.children[head].add(tail);
            break;
        }
        break;
    }
    return;
};
// Find an item in the Trie.
TrieNode.prototype.find = function find(item) {
    'use strict';
    var head,
        tail;
    
    switch (item.length) {
    case 0:
        // Base Case
        if (this.word) {
            return true;
        } else {
            return false;
        }
    default:
        // Recursive Case
        head = item.charAt(0);
        tail = item.substring(1, item.length);
        
        switch (this.children[head]) {
        case null:
        case undefined:
            // No node created yet!
            // Create, then recursively add.
            return false;
        default:
            // A node exists.
            // Recursively add.
            return this.children[head].find(tail);
        }
    }
};

main();