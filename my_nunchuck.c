#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input-polldev.h>
#include <linux/delay.h>

struct nunchuk_dev {
struct input_polled_dev *polled_input;
struct i2c_client *i2c_client;
};


/****************************************************************************************/
/*																						*/
/*																						*/
/*							Read Nunchuk Register										*/
/*																						*/
/****************************************************************************************/
int read_nunchuk_register(char *buffy,struct i2c_client *client)
{
	int error;
	/*Put 10 ms delay*/
	mdelay(10);
	error =  i2c_master_send(client,0x00, 1);
	if(error != 1)
	{
		dev_err(&client->dev, "Failed to send data. \n");
		return -EIO;
	}
	/*Put 10 ms delay*/
	mdelay(10);
	error = i2c_master_recv(client,buffy, 6);	
	return 0;
}


/****************************************************************************************/
/*																						*/
/*																						*/
/*							Input FreameWork Polling Function							*/
/*																						*/
/****************************************************************************************/
void nunchuk_button_polling(struct input_polled_dev *dev)
{
/* The way to do this is simple */
/* Send I2c commands to read 'C' and 'Z' button */
/* Based on the data received somehow trigger the event.*/

struct nunchuk_dev *nunchuk;
nunchuk = dev->private;

}


/****************************************************************************************/
/*																						*/
/*																						*/
/*							DeviceDriver Polling Function								*/
/*																						*/
/****************************************************************************************/
static int nunchuck_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
/* initialize device */
/* register to a kernel framework */
/*i2c_set_clientdata(client, <private data>);*/


/* This is subclass of input_dev and is used for devices which */
/* do not have interrupt mechanism and rely on polling.        */
struct input_polled_dev *polled_input;

struct nunchuk_dev *nunchuk;

/* This for Input device driver */
/* This is used just to point inside input_polled_dev structure */
struct input_dev *input;


int error;

char init_buffer[] = { 0xF0, 0x55, 0xFB, 0x00 };

char buffy[6];


pr_info("Wazzup people, Probe method called \n");

/* I think the first input is only for logging purpose */
nunchuk = devm_kzalloc(&client->dev, sizeof(struct nunchuk_dev), GFP_KERNEL);

if (!nunchuk) 
{
    dev_err(&client->dev, "Failed to allocate memory\n");
    return -ENOMEM;
}

/* Below needs to be defined for event handler*/
/*struct input_handler*/

/*Allocate memory to polling input*/
polled_input = input_allocate_polled_device();

if(!polled_input)
{
	return -ENOMEM;
}

/* Map to struct nunchuk_dev */
/* This is simple */
nunchuk->i2c_client = client;

/* This is simple */
nunchuk->polled_input = polled_input;

/* What the hell is this*/
/* Header says: private: private driver data */
/* This is kind of a mapping so that the input framework has information of what the overall thing is all about... */
/* This information is used when creating polling function (nunchuk_button_polling) */
polled_input->private = nunchuk;

/* What is this */
i2c_set_clientdata(client, nunchuk);

/* This is simple */
input = polled_input->input;

/* What is this */
/*Maps the device structure */
/*device structure has below info: */
/*i/os, IRqs blah blah*/
input->dev.parent = &client->dev;

/* Stuff that requires to be populated */
/*This should match the struct driver one */
input->name = "nunchuck";

/*I can guess what this is*/
input->id.bustype = BUS_I2C;

/* See http://lxr.free-electrons.com/source/include/uapi/linux/input.h#L178 */
/* Basically for button press EV_KEY even is generated */
set_bit(EV_KEY, input->evbit);

/* Since there are two buttons in Nunchuck 'C' and 'Z' so these are registers*/
set_bit(BTN_C, input->keybit);
set_bit(BTN_Z, input->keybit);

/*Scope for detecting accelerometer change can be added later.*/

/*I know this */
error = input_register_polled_device(polled_input);

if(error)
{
	goto err_free_nunchuck_p_dev;
}

/* The nunchuck needs to be initialised */
/* This is how you do it */
/* Mnual at http://web.engr.oregonstate.edu/~sullivae/ece375/pdf/nunchuk.pdf */

/* First send 0xF0 and 0x55 to initialize first register */
error =  i2c_master_send(nunchuk->i2c_client,init_buffer, 2);

if(error != 2)
{
    dev_err(&client->dev, "Failed to send first initialization command. \n");
    goto err_free_nunchuck_p_dev;
}

/*Wait for 1 Millisecond */
udelay(1000);

/* Then send 0xFB and 0x00 to initialize second register */
error =  i2c_master_send(nunchuk->i2c_client,&init_buffer[2], 2);

if(error != 2)
{
    dev_err(&client->dev, "Failed to send second initialization command. \n");
    goto err_free_nunchuck_p_dev;
}

/* Called twice as Nunchuk updates its register once you have read it. */
read_nunchuk_register(buffy,nunchuk->i2c_client);

read_nunchuk_register(buffy,nunchuk->i2c_client);

 /* Now can do stuff with buffy */
 /* Lets just output the status of Z button which is bit 0 of Byte 5 */
 /* If the value is 0 then the button is pressed else the button is released */
 /* if(!bit(buffy[5],0)) */
 if(!(buffy[5] & 0x01))
  {
	pr_info("Button Z pressed \n");
 }
 else
 {
	pr_info("Button Z released \n");
 }
 

return 0;

err_free_nunchuck_p_dev: 
input_free_polled_device(polled_input);
return -1;
}

/****************************************************************************************/
/*																						*/
/*																						*/
/*							DeviceDriver Remove Function								*/
/*																						*/
/****************************************************************************************/
static int nunchuck_remove(struct i2c_client *client)
{
/*<private data> = i2c_get_clientdata(client);*/

struct nunchuk_dev *nunchuk;
nunchuk = i2c_get_clientdata(client);


/*Unregister the input device*/
input_unregister_polled_device(nunchuk->polled_input);
/*De-allocate the memory of input polled*/
input_free_polled_device(nunchuk->polled_input);



pr_info("Wazzup people, Remove method called \n");
/* unregister device from kernel framework */
/* shut down the device */
return 0;
}


/****************************************************************************************/
/*																						*/
/*																						*/
/*							Device Id Mapping Table										*/
/*																						*/
/****************************************************************************************/
static const struct i2c_device_id nunchuck_id[] = {
	{ "nunchuck", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, nunchuck_id);


/****************************************************************************************/
/*																						*/
/*																						*/
/*							I2C DeviceDriver Structure Populated						*/
/*																						*/
/****************************************************************************************/
static struct i2c_driver nunchuck_driver = {
         .driver         = {
                 .name   = "nunchuck",
                 .owner  = THIS_MODULE,
         },
         .probe         = nunchuck_probe,
	 .remove	= nunchuck_remove,
         .id_table      = nunchuck_id,
 };
 
 module_i2c_driver(nunchuck_driver);

MODULE_LICENSE("GPL");
