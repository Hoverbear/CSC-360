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
    var root = new TrieNode('');
    
    // Encourage STDIN to behave desirably.
    process.stdin.resume();
    process.stdin.setEncoding('utf8');
    
    // Populate the Trie from STDIN.
    // This will be one large file.
    process.stdin.on('data', function (data) {
        data.split("\\n").map(root.add, root);
    });
    
    // Once we've read for STDIN, start searching through words for words that exist when spelled backwards.
    process.stdin.on('end', function () {
        console.log(JSON.stringify(root, null, " "));
        console.log(root.find("test"));
    });
    
}

// Trie Node.
TrieNode = function (value) {
    'use strict';
    
    this.value = value;
    this.children = {};
    this.words = 0;
    
    return this;
};
// Add a new item to the Trie.
TrieNode.prototype.add = function (item) {
    'use strict';
    var head,
        tail;

    switch (item.length) {
    case 0:
        // Base Case
        this.words += 1;
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
            this.children[head].add(tail);
            break;
        }
        break;
    }
    return;
};
// Find an item in the Trie.
TrieNode.prototype.find = function (item) {
    'use strict';
    var head,
        tail;
    
    switch (item.length) {
    case 0:
        // Base Case
        if (this.words !== 0) {
            return true;
        }
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
            return false;
        default:
            // A node exists.
            // Recursively add.
            return this.children[head].find(tail);
        }
    }
};

main();