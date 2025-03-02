#include "USB.h"

// STATIC //

static libusb_context* context = NULL;

//  PRIV FUNCS  //

void init_context();

////////////////////////
//  PUBLIC FUNCTIONS  //
////////////////////////


USB_handle* USB_init(int interface, int productID, int vendorID)
{
  init_context();


  USB_handle handle;

  handle.interface = interface;
  handle.productID = productID;
  handle.vendorID = vendorID;

  libusb_device **list;
  ssize_t cnt = libusb_get_device_list(context, &list);
  if (cnt < 0)
    return NULL;

  bool found = false;
  for (int i = 0; i < cnt; i++)
  {
    libusb_device* device = list[i];
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(device, &desc);
    if (desc.idVendor == handle.vendorID && desc.idProduct == handle.productID)
    {
      handle.device = device;
      found = true;
      break;
    }
  }

  libusb_free_device_list(list, 1);
  if (!found)
    return NULL;

  handle.handle = libusb_open_device_with_vid_pid(context, handle.vendorID, handle.productID);
  if (!handle.handle)
    return NULL;

  libusb_detach_kernel_driver(handle.handle, handle.interface);
  libusb_claim_interface(handle.handle, handle.interface);

  struct libusb_config_descriptor* config;
  const struct libusb_interface* inter;
  const struct libusb_interface_descriptor* inter_desc;
  const struct libusb_endpoint_descriptor* end_desc;

  libusb_get_config_descriptor(handle.device, 0, &config);
  inter = &config->interface[handle.interface];

  inter_desc = &inter->altsetting[0];
  end_desc = &inter_desc->endpoint[0];
  handle.endpoint_out_address = end_desc->bEndpointAddress;
  end_desc = &inter_desc->endpoint[1];
  handle.endpoint_in_address = end_desc->bEndpointAddress;

  USB_handle* hp = calloc(1, sizeof(USB_handle));
  memcpy(hp, &handle, sizeof(USB_handle));
  return hp;
}

void USB_free(USB_handle* handle)
{
  if (!handle)
    return;

  if (handle->handle)
  {
    if (handle->interface)
      libusb_release_interface(handle->handle, handle->interface);
    libusb_close(handle->handle);
  }

  free(handle);
}

void USB_destroy_context()
{
  if (!context)
    return;

  libusb_exit(context);
  context = NULL;
}

int USB_read_data(USB_handle* handle, void* data, int size)
{
  if(size < 0 || !handle)
  {
    return 0;
  }

  int actual = 0; //used to find out how many bytes were written
  int r = libusb_bulk_transfer(handle->handle, (handle->endpoint_in_address | LIBUSB_ENDPOINT_IN), data, size, &actual, 7);
  //printf("Primer: %d\n", actual);

  return actual; 
}

int USB_send_data(USB_handle* handle, void* data, int size)
{
  if (size < 0 || !handle)
    return 0;

  int actual;
  int r = libusb_bulk_transfer(handle->handle, (handle->endpoint_out_address | LIBUSB_ENDPOINT_OUT), data, size, &actual, 7);

  return actual;
}

//  PRIV FUNCS  //

void init_context()
{
  if (context)
    return;

  libusb_init(&context);
}
