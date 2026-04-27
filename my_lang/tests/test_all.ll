// Test file for LiteLogic

x = 10;
y = 3.14;
c = 'A';
s = "Hello World";

print(x);
print(y);
print(c);
print(s);

// test conditions
if (x > 5 AND y < 4.0) {
    print("Math works!");
}

// test loop
count = 0;
while (count < 3) {
    print(count);
    count = count + 1;
}

// test for loop
for (i = 0; i < 3; i = i + 1) {
    print(i * 10);
}

// test functions
def add(a, b) {
    return a + b;
}

res = add(10, 20);
print("Result of func:");
print(res);

def greet(name) {
    print("Welcome " + name);
}

greet("Bob");

print(len("test string"));
print(str(100) + " points");
