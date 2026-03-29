#include "../RBTree.hpp"
#include <iostream>

int main() {
    freopen("1.in", "r", stdin);
    freopen("1.out", "w", stdout);
    sjtu::RBTree<long long> s[25];
    sjtu::RBTree<long long>::iterator it;
    int op, lst=0, it_a=-1, valid = 0, cnt=1;
    std::cout << "begin" << '\n';
    while (scanf("%d", &op) != EOF) {
        long long a, b, c;
        switch (op) {
            case 0: {
                scanf("%lld%lld", &a, &b);
                auto p=s[a].emplace(b);
                if(p.second) {
                	it_a = a;
                    it = p.first;
                    valid = 1;
                }
                std::cout << "0" << "\n";
                break;
            }
            case 1:
                scanf("%lld%lld", &a, &b);
                if (valid && it_a==a && *it == b)valid = 0;
                s[a].erase(b);
                std::cout << "1" << "\n";
                break;
            case 2:
                scanf("%lld", &a);
                s[++lst] = s[a];
                std::cout << "2" << "\n";
                break;
            case 3: {
                scanf("%lld%lld", &a, &b);
                auto it2 = s[a].find(b);
                if (it2 != s[a].end()) {
                    printf("true\n");
                    it_a = a;
                    it = it2;
                    valid = 1;
                } else
                    printf("false\n");
                cnt++;
                std::cout << "3" << "\n";
                break;
            }
            case 4:
                scanf("%lld%lld%lld", &a, &b, &c);
                printf("%d\n", s[a].range(b, c));
                cnt++;
                std::cout << "4" << "\n";
                break;
            case 5:
                if (valid){
                    auto it2=it;
                    if (it==--it2)valid = 0;
                }
                if (valid)
                    printf("%lld\n", *(--it));
                else
                    printf("-1\n");
                cnt++;
                std::cout << "5" << "\n";
                break;
            case 6:
                if (valid) {
                    auto it2=++it;
                    if (it==++it2)valid = 0;
                    else printf("%lld\n", *it);
                }
                if (!valid)
                    printf("-1\n");
                cnt++;
                std::cout << "6" << "\n";
                break;
        }
    }
    return 0;
}
