#include <stdio.h>
#include "sai.h"

const char *test_profile_get_value(
    _In_ sai_switch_profile_id_t profile_id,
    _In_ const char *variable)
{
    return 0;
}

int test_profile_get_next_value(
    _In_ sai_switch_profile_id_t profile_id,
    _Out_ const char **variable,
    _Out_ const char **value)
{
    return -1;
}

const service_method_table_t test_services = {
    test_profile_get_value,
    test_profile_get_next_value};

int main()
{
    sai_status_t status;
    sai_switch_api_t *switch_api;
    sai_lag_api_t *lag_api;
    sai_object_id_t lag_id[7];
    sai_object_id_t lag1_member_id[17];
    sai_object_id_t lag3_member_id[17];
    sai_attribute_t attrs[2];
    sai_switch_notification_t notifications;
    sai_object_id_t port_list[32];



    status = sai_api_initialize(0, &test_services);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to initialize test services, status=%d\n", status);
        return 1;
    }

    status = sai_api_query(SAI_API_SWITCH, (void **)&switch_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to query SWITCH API, status=%d\n", status);
        return 1;
    }

    status = switch_api->initialize_switch(0, "HW_ID", 0, &notifications);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to initilize switch, status=%d\n", status);
        return 1;
    }

    status = sai_api_query(SAI_API_LAG, (void **)&lag_api);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to query LAG API, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_SWITCH_ATTR_PORT_LIST;
    attrs[0].value.objlist.list = port_list;
    attrs[0].value.objlist.count = 32;
    status = switch_api->get_switch_attribute(1, attrs);
    for (int32_t ii = 0; ii < attrs[0].value.objlist.count; ii++)
    {
        printf("Port #%d OID: 0x%lX\n", ii, attrs[0].value.objlist.list[ii]);
    }

    //1. Check that LAG can be created (base case).
    status = lag_api->create_lag(&lag_id[0], 0, NULL);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create a LAG1, status=%d\n", status);
        return 1;
    }

    //2. Check that port can be added to LAG with correct parameters
    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 1;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 1;

    status = lag_api->create_lag_member(&lag1_member_id[0], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #1, status=%d\n", status);
        return 1;
    }
    
    //3. Check that appropriate error message is displayed when trying to create lag member with missing/invalid PORT_ID
    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 1;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = NULL;

    status = lag_api->create_lag_member(&lag1_member_id[1], 2, attrs);
    if (status != SAI_STATUS_INVALID_PARAMETER)
    {
        printf("Incorrect error message for missing PORT_ID, status=%d\n", status);
        return 1;
    }
    //4. Check that appropriate error message is displayed when trying to create lag member with missing/invalid LAG_ID
    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = NULL;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 1;

    status = lag_api->create_lag_member(&lag1_member_id[1], 2, attrs);
    if (status != SAI_STATUS_INVALID_PARAMETER)
    {
        printf("Incorrect error message for missing LAG_ID, status=%d\n", status);
        return 1;
    }
    
    //5. Check that multiple LAGs can be created (up to 5)
    // Create LAG2
    status = lag_api->create_lag(&lag_id[1], 0, NULL);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create a LAG2, status=%d\n", status);
        return 1;
    }

    // Create LAG3
    status = lag_api->create_lag(&lag_id[2], 0, NULL);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create a LAG3, status=%d\n", status);
        return 1;
    }

    // Create LAG4
    status = lag_api->create_lag(&lag_id[3], 0, NULL);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create a LAG4, status=%d\n", status);
        return 1;
    }

    // Create LAG5
    status = lag_api->create_lag(&lag_id[4], 0, NULL);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create a LAG5, status=%d\n", status);
        return 1;
    }

    //6. Check that appropriate error message is returned when trying to create 6th LAG
    // Create LAG6
    status = lag_api->create_lag(&lag_id[5], 0, NULL);
    if (status != SAI_STATUS_INVALID_PARAMETER)
    {
        printf("Incorrect response for creating LAG that is out of capacity, status=%d\n", status);
        return 1;
    }

    //7. Check that last created LAG can be removed
    // Remove LAG5
    status = lag_api->remove_lag(lag_id[4]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to REMOVE LAG5, status=%d\n", status);
    }

    //8. Check that previously created LAG can be removed (out of creation order)
    //9. Check that empty LAG can be removed
    // Remove LAG2
    status = lag_api->remove_lag(lag_id[1]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to REMOVE LAG2, status=%d\n", status);
    }

    //10. Check that LAG with ports added can be removed
    // Remove LAG1
    status = lag_api->remove_lag(lag_id[0]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to REMOVE LAG1, status=%d\n", status);
    }

    //11. Check that multiple ports can be added to the LAG(up to 16)

    // Create LAG3 MEMBERS
    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 1;

    status = lag_api->create_lag_member(&lag3_member_id[0], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #1, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 2;

    status = lag_api->create_lag_member(&lag3_member_id[1], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #2, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 3;

    status = lag_api->create_lag_member(&lag3_member_id[2], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #3, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 4;

    status = lag_api->create_lag_member(&lag3_member_id[3], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #4, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 5;

    status = lag_api->create_lag_member(&lag3_member_id[4], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #5, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 6;

    status = lag_api->create_lag_member(&lag3_member_id[5], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #6, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 7;

    status = lag_api->create_lag_member(&lag3_member_id[6], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #7, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 8;

    status = lag_api->create_lag_member(&lag3_member_id[7], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #8, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 9;

    status = lag_api->create_lag_member(&lag3_member_id[8], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #9, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 10;

    status = lag_api->create_lag_member(&lag3_member_id[9], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #10, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 11;

    status = lag_api->create_lag_member(&lag3_member_id[10], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #11, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 12;

    status = lag_api->create_lag_member(&lag3_member_id[11], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #12, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 13;

    status = lag_api->create_lag_member(&lag3_member_id[12], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #13, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 14;

    status = lag_api->create_lag_member(&lag3_member_id[13], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #14, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 15;

    status = lag_api->create_lag_member(&lag3_member_id[14], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #15, status=%d\n", status);
        return 1;
    }

    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 16;

    status = lag_api->create_lag_member(&lag3_member_id[15], 2, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to create LAG_MEMBER #16, status=%d\n", status);
        return 1;
    }

    //12. Check that correct error message is returned when trying to ad 17th port to LAG
    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 17;

    status = lag_api->create_lag_member(&lag3_member_id[16], 2, attrs);
    if (status != SAI_STATUS_INVALID_PARAMETER)
    {
        printf("Incorrect message for adding LAG_MEMBER exceeding limit , status=%d\n", status);
        return 1;
    }

    //18. Check that apropriate error message is returned when trying to add already existing port to LAG
    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.u8 = 3;

    attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    attrs[1].value.u8 = 16;

    status = lag_api->create_lag_member(&lag3_member_id[15], 2, attrs);
    if (status != SAI_STATUS_ITEM_ALREADY_EXISTS)
    {
        printf("Incorrect error message for adding already existing port, status=%d\n", status);
        return 1;
    }

    //13. Check that last added port can be removed from the LAG
    // Remove LAG MEMBER 16
    status = lag_api->remove_lag_member(lag3_member_id[15]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }

    //14. Check that previously added port (not the last one) can be removed from the LAG
    status = lag_api->remove_lag_member(lag3_member_id[0]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }

    status = lag_api->remove_lag_member(lag3_member_id[1]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }

    //16. Check that correct LAG_MEMBER attributes are returned for LAG_MEMBERs
    // Get LAG ID of LAG MEMBER
    attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;

    status = lag_api->get_lag_member_attribute(lag3_member_id[2], 1, attrs);
    if (status != SAI_STATUS_SUCCESS || &attrs[0].value != lag_id[2])
    {
        printf("Incorrect value of LAG_ID for LAG_MEMBER, status=%d\n", status);
    }

    // Get PORT ID of LAG MEMBER
    attrs[0].id = SAI_LAG_MEMBER_ATTR_PORT_ID;

    status = lag_api->get_lag_member_attribute(lag3_member_id[3], 1, attrs);
    if (status != SAI_STATUS_SUCCESS || &attrs[0].value != lag3_member_id[3])
    {
        printf("Incorrect value of LAG_PORT_ID for LAG_MEMBER, status=%d\n", status);
    }
    
    //17. Check that correct PORT_LIST is returned for lag
    // check_port_list is a function to be implemented for comparison
    // Get PORT LIST of LAG 3
    attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    attrs[0].value.objlist.list = port_list;
    attrs[0].value.objlist.count = 16;
    int expected[] = {3,4,5,6,7,8,9,10,11,12,13,14,15};
    status = lag_api->get_lag_attribute(lag_id[2], 1, attrs);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a LAG ATTRIBUTE, status=%d\n", status);
    }
    if (check_port_list(attrs, expected)){
        printf("Incorrect value of PORT_LIST, attrs=%d\n", attrs);
    }


    //15. Check that all ports can be removed from the LAG
    status = lag_api->remove_lag_member(lag3_member_id[2]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
    
    status = lag_api->remove_lag_member(lag3_member_id[3]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[4]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[5]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[6]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[7]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[8]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[9]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[10]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[11]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[12]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[13]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }
     status = lag_api->remove_lag_member(lag3_member_id[14]);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("Failed to get a remove LAG MEMBER , status=%d\n", status);
    }

    switch_api->shutdown_switch(0);
    status = sai_api_uninitialize();

    return 0;
}