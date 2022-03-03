#include <stdio.h>
#include <stdlib.h>

typedef struct ListNode {
	int val;
	struct ListNode *next;
}ListNode;

void display(ListNode *p) {
	ListNode *temp=p;
	while (temp) {
		printf("%d ",temp->val);
		temp=temp->next;
    }
	printf("\n");
}


struct ListNode* addTwoNumbers(struct ListNode* l1, struct ListNode* l2){
	int i=0;
    int over=0;
    int tmp_val=0;
	struct ListNode* result_head=NULL,*result_tail=NULL;

    while(1){
        if(l1!=NULL&&l2!=NULL){
            tmp_val=l1->val+l2->val+over;
            if(tmp_val>9){
                tmp_val=tmp_val%10;
                over=1;
            }else{
                over=0;
            }
            l1=l1->next;
            l2=l2->next;
        }else if(l2==NULL&&l1!=NULL){
			if((l1->val+over)>9){
				tmp_val=(l1->val+over)%10;
				over=1;
			}else{
				tmp_val=l1->val+over;
				over=0;
			}
            l1=l1->next;
        }else if(l1==NULL&&l2!=NULL){
			if((l2->val+over)>9){
				tmp_val=(l2->val+over)%10;
				over=1;
			}else{
				tmp_val=l2->val+over;
				over=0;
			}
            l2=l2->next;
        }

		if(!result_head){
			result_head=result_tail=(struct ListNode*)malloc(sizeof(struct ListNode));
			result_tail->val=tmp_val;
			result_tail->next=NULL;
		}else{
			result_tail->next=(struct ListNode*)malloc(sizeof(struct ListNode));
			result_tail->next->val=tmp_val;
			result_tail=result_tail->next;
			result_tail->next=NULL;
		}

		if(l1==NULL&&l2==NULL){
			if(over==1){
				result_tail->next=(struct ListNode*)malloc(sizeof(struct ListNode));
				result_tail->next->val=1;
				result_tail=result_tail->next;
				result_tail->next=NULL;
			}
			break;
		}
    }

    return result_head;
}



void main(void){
	int i=0;
	int a[]={1,4,5};
	int b[]={1,3,4};
	int c[]={2,6};
	ListNode* l1_head=NULL,*l1_tail=NULL;
	ListNode* l2_head=NULL,*l2_tail=NULL;
	ListNode* l3_head=NULL,*l3_tail=NULL;
	for(i=0;i<sizeof(a)/sizeof(a[0]);i++){
		if(!l1_head){
			l1_head=l1_tail=(ListNode*)malloc(sizeof(ListNode));
			l1_tail->val=a[i];
			l1_tail->next=NULL;
		}else{
			l1_tail->next=(ListNode*)malloc(sizeof(ListNode));
			l1_tail->next->val=a[i];
			l1_tail=l1_tail->next;
			l1_tail->next=NULL;
		}
	}
	//display(l1_head);

	for(i=0;i<sizeof(b)/sizeof(b[0]);i++){
		if(!l2_head){
			l2_head=l2_tail=(ListNode*)malloc(sizeof(ListNode));
			l2_tail->val=b[i];
			l2_tail->next=NULL;
		}else{
			l2_tail->next=(ListNode*)malloc(sizeof(ListNode));
			l2_tail->next->val=b[i];
			l2_tail=l2_tail->next;
			l2_tail->next=NULL;
		}
	}
	//display(l2_head);

	for(i=0;i<sizeof(c)/sizeof(c[0]);i++){
		if(!l3_head){
			l3_head=l3_tail=(ListNode*)malloc(sizeof(ListNode));
			l3_tail->val=c[i];
			l3_tail->next=NULL;
		}else{
			l3_tail->next=(ListNode*)malloc(sizeof(ListNode));
			l3_tail->next->val=a[i];
			l3_tail=l3_tail->next;
			l3_tail->next=NULL;
		}
	}
	display(l3_head);

	ListNode* result=addTwoNumbers(l1_head,l2_head);
	display(result);

#if 0
	ListNode* l2=(ListNode*)malloc(sizeof(ListNode));
	ListNode* temp2=l2;
	for(i=0;i<sizeof(b)/sizeof(b[0]);i++){
		ListNode *tmp2 = (ListNode*)malloc(sizeof(ListNode));
		tmp2->val=b[i];
		tmp2->next=NULL;
		temp2->next=tmp2;
		//printf("temp %d \n",l1->val);
		temp2=temp2->next;
	}
	display(l2);

	ListNode* result=addTwoNumbers(l1,l2);
	display(result);
#endif
	return;
}

