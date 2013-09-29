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
        var words = root.iterator(),
            word;
        while (true) {
            word = words.next();
            switch (word) {
            case false:
                // Out of words.
                process.exit(0);
                return;
            default:
                // We have more words.
                console.log(word);
            }
            
        }
    });
    
}

// Trie Node.
TrieNode = function TrieNode(value, words) {
    'use strict';
    
    this.value = value;
    this.children = {};
    if (words === null) {
        this.words = words;
    } else {
        this.words = 0;
    }

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
            this.words += 1;
            this.children[head] = new TrieNode(head, this.words);
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
// Iterate over the Trie.
TrieNode.prototype.iterator = function iterator(path) {
    'use strict';
    var root = this,
        child,
        head = '',
        tail = Object.keys(this.children),
        childIterator,
        remaining = this.words,
        next;

    next = function next() {
        var iteratee;
        
        if (remaining === root.words) {
            // This is a word, we need it next!
            console.log(String(path) === String(''));
            remaining -= 1;
            if (String(path) !== String('')) {
                return path;
            } else {
                return this.next();
            }
        } else if (remaining === 0) {
            return false;
        } else {
            // This is not a word, or has already been consumed.
            // We need to continue.
            head = tail.shift();
            switch (head) {
            case undefined:
                // TODO: Bug where we miss Z?
                // We've exhausted our list.
                return false;
            }
//            console.log("HEAD IS: " + head);
            console.log("remiaining: " + remaining);
            child = root.children[head];
            console.log("Child: " + JSON.stringify(child, false, " "));
//            console.log("Root: " + JSON.stringify(root, false, " "));
            switch (child) {
            case undefined:
                return false;
            default:
                // This is a sub-Trie that will also iterate.
                childIterator = child.iterator();
                path += head;
                console.log("Path " + path);
                iteratee = childIterator.next();
                console.log("ITeratee is " + iteratee);
                switch (iteratee) {
                case false:
                    // This sub-trie is done.
                    return false;
                default:
                    return iteratee;
                }
            }
        }
    };
    
    return {
        next: next
    };
};

main();