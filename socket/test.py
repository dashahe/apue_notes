from simple_request import simple_requests

if __name__ == '__main__':
    html = simple_requests().get('www.baidu.com').html
    print(html)