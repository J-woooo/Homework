typedef struct tree_node* tree_pointer;
typedef struct tree_node {
	int key;
	int data;
	tree_pointer left;
	tree_pointer right;
}tree_node;
tree_pointer root = NULL;   //BST의 root를 가리키는 포인터(전역변수)


void bst_insert(int key, char data);   //트리에 (key,data)자료 삽입
char bst_search(int key);   //트리에서 키값이 key인 자료를 검색, data를 반환
void bst_show_inorder(tree_pointer ptr);   //트리의 자료들을 inorder로 출력
