package com.bsh.test;

//http://developer.51cto.com/art/201112/306366.htm
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.util.Iterator;

public class Service2 {
	static int SIZE = 10;
    static InetSocketAddress ip = new InetSocketAddress("211.88.253.7", 5566);
    static CharsetEncoder encoder = Charset.forName("UTF8").newEncoder();
    static CharsetDecoder decoder = Charset.forName("UTF8").newDecoder();
 
    static class Message implements Runnable {
        protected String name;
 
        public Message(String index) {
            this.name = index;
        }
 
        public void run() {
            try {
                //打开Socket通道   
                SocketChannel client = SocketChannel.open();
                //设置为非阻塞模式   
                client.configureBlocking(false);
                //打开选择器   
                Selector selector = Selector.open();
                //注册连接服务端socket动作   
                client.register(selector, SelectionKey.OP_CONNECT);
                //连接   
                client.connect(ip);
                //分配内存   
                ByteBuffer buffer = ByteBuffer.allocate(8 * 1024);
 
                _FOR: for (;;) {
                    selector.select();
                    Iterator<SelectionKey> iter = selector.selectedKeys().iterator();
 
                    while (iter.hasNext()) {
                        SelectionKey key = (SelectionKey) iter.next();
                        iter.remove();
                        if (key.isConnectable()) {
                            SocketChannel channel = (SocketChannel) key.channel();
                            if (channel.isConnectionPending()) channel.finishConnect();
                            
                            channel.write(encoder.encode(CharBuffer.wrap(name)));
 
                            channel.register(selector, SelectionKey.OP_READ);
                        } else if (key.isReadable()) {
                            SocketChannel channel = (SocketChannel) key.channel();
                            
                            int count = channel.read(buffer);
                            String msg = "";
                            if (count > 0) {
                                buffer.flip();
                                msg = decoder.decode(buffer).toString();
                                System.out.println(msg);
                                
                                buffer.clear();
                            } else {
                                client.close();
                                break _FOR;
                            }
                        }
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    
    public static void send(String msg) {
    	
    }
 
    public static void main(String[] args) throws IOException {
    	String name = "bsh";
		String password = "123";
    	new Thread(new Message("{\"action\":\"login\",\"name\":\"" + name + "\",\"password\":\"" + password + "\"}")).start();
       
//        String names[] = new String[SIZE];
// 
//        for (int index = 0; index < SIZE; index++) {
//            names[index] = "jeff[" + index + "]";
//            new Thread(new Message(names[index])).start();
//        }
       
    }
}