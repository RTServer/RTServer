package com.bsh.test;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;
import java.util.Iterator;

public class NIOSocketClient2 {
	static int SIZE = 10;
    static InetSocketAddress ip = new InetSocketAddress("211.88.253.7", 5566);
    static CharsetEncoder encoder = Charset.forName("UTF8").newEncoder();
 
    static class Message implements Runnable {   
        protected String name;
        String msg = "";
 
        public Message(String index) {   
            this.name = index;
        }
 
        public void run() {   
            try {   
                long start = System.currentTimeMillis();
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
                int total = 0;
 
                _FOR: for (;;) {   
                    selector.select();
                    Iterator<SelectionKey> iter = selector.selectedKeys().iterator();
 
                    while (iter.hasNext()) {   
                        SelectionKey key = (SelectionKey) iter.next();
                        iter.remove();
                        if (key.isConnectable()) {   
                            SocketChannel channel = (SocketChannel) key   
                                    .channel();
                            if (channel.isConnectionPending())   
                                channel.finishConnect();
                            channel   
                                    .write(encoder   
                                            .encode(CharBuffer.wrap(name)));
 
                            channel.register(selector, SelectionKey.OP_READ);
                        } else if (key.isReadable()) {   
                            SocketChannel channel = (SocketChannel) key   
                                    .channel();
                            int count = channel.read(buffer);
                            if (count > 0) {   
                                total += count;
                                buffer.flip();
 
                                while (buffer.remaining() > 0) {   
                                    byte b = buffer.get();
                                    msg += (char) b;
                                       
                                }
 
                                buffer.clear();
                            } else {   
                                client.close();
                                break _FOR;
                            }
                        }
                    }
                }
                double last = (System.currentTimeMillis() - start) * 1.0 / 1000;
                System.out.println(msg + "used time :" + last + "s.");
                msg = "";
            } catch (IOException e) {   
                e.printStackTrace();
            }
        }
    }
 
    public static void main(String[] args) throws IOException {   
       
        String names[] = new String[SIZE];
 
        for (int index = 0; index < SIZE; index++) {   
            names[index] = "jeff[" + index + "]";
            new Thread(new Message(names[index])).start();
        }
       
    }
}
