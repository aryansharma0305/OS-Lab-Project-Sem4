#ifndef STRUCTS_H
#define STRUCTS_H


struct orders
{
    int orderID;
    char name[50];
    char phone[11];
    char email[50];
    int tableID;
    int menuItemsIDAndQuantity[100];   // I am thinking of puting the menu item ID and quantity in the same array, like [menuItemID1, quantity1, menuItemID2, quantity2,...]
    // also I am assuming a person can only order 50 unique items in one orde
    char orderTime[6]; // HHMMFormat
    char orderDate[9]; // DDMMYYYYFormat    
    float totalBill;
    
};





struct menu
{
    int itemID;
    char itemName[50];
    float price;
};





struct tables
{
    int tableID;
    int capacity;
    int isOccupied; // 0 for false, 1 for true
};





struct msg_buffer{

    long msg_type;  // This will signify the type of query like read, delete, update, insert.
    
    long load_type; // Since there can be multiple tables?, this will signify the table type like order, menu, table. 

    long reply_to; // something like uuid - will see later what to do about it.

    union 
    {        
        struct orders orders;
        struct menu menu;
        struct tables tables;   
    }payload;

};

#endif