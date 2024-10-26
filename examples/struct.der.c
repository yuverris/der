struct No9ta {
int x;
int y;
};
;
int no9_plus(struct No9ta n){
	return n.x + n.y;
}
;
int main(){
	struct No9ta f = {.x = 78,.y = 4};
	return no9_plus(f);
}
;
